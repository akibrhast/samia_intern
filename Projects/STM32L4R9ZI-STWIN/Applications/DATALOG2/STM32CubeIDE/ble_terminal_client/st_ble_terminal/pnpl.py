from __future__ import annotations

import json
import struct
from dataclasses import dataclass, field
from typing import Any


@dataclass
class StreamField:
    name: str
    fmt: str
    unit: str | None = None
    elements: int = 1
    channels: int = 1
    multiply_factor: float = 1.0
    odr: int | None = None
    minimum: float | None = None
    maximum: float | None = None


@dataclass
class RawStream:
    stream_id: int
    component: str
    fields: list[StreamField] = field(default_factory=list)


@dataclass
class SensorComponent:
    name: str
    enabled: bool | None = None
    stream_id: int | None = None


def compact_json(value: dict[str, Any]) -> str:
    return json.dumps(value, separators=(",", ":"))


def command_get_status(target: str = "all") -> str:
    return compact_json({"get_status": target})


def command_log_controller_status() -> str:
    return compact_json({"get_status": "log_controller"})


def command_set_time(dt: str) -> str:
    return compact_json({"log_controller*set_time": {"datetime": dt}})


def command_start_ble() -> str:
    return compact_json({"log_controller*start_log": {"interface": 2}})


def command_stop() -> str:
    return compact_json({"log_controller*stop_log": {}})


def command_sensor_enable(sensor_name: str, enabled: bool) -> str:
    return compact_json({sensor_name: {"enable": enabled}})


def parse_json_payload(payload: bytes) -> dict[str, Any] | None:
    raw = payload.rstrip(b"\x00")
    if len(raw) >= 2:
        declared = int.from_bytes(raw[:2], "big")
        if declared == len(raw) - 2:
            raw = raw[2:]
    try:
        return json.loads(raw.decode("utf-8"))
    except (UnicodeDecodeError, json.JSONDecodeError):
        return None


def _device_components(status: dict[str, Any]) -> list[dict[str, Any]]:
    if "devices" in status:
        devices = status.get("devices") or []
        if devices and isinstance(devices[0], dict):
            components = devices[0].get("components")
            if isinstance(components, list):
                return [c for c in components if isinstance(c, dict)]
    components = status.get("components")
    if isinstance(components, list):
        return [c for c in components if isinstance(c, dict)]
    return []


def parse_components(status: dict[str, Any]) -> tuple[dict[str, SensorComponent], dict[int, RawStream]]:
    sensors: dict[str, SensorComponent] = {}
    streams: dict[int, RawStream] = {}

    for component in _device_components(status):
        for name, body in component.items():
            if not isinstance(body, dict):
                continue

            enabled = body.get("enable")
            st_ble_stream = body.get("st_ble_stream")
            sensor = SensorComponent(
                name=name,
                enabled=enabled if isinstance(enabled, bool) else None,
                stream_id=None,
            )

            if isinstance(st_ble_stream, dict):
                stream_id = st_ble_stream.get("id")
                if isinstance(stream_id, int):
                    sensor.stream_id = stream_id
                    stream = streams.setdefault(stream_id, RawStream(stream_id, name))
                    for field_name, field_body in st_ble_stream.items():
                        if field_name in {"id", "custom", "labels"} or not isinstance(field_body, dict):
                            continue
                        fmt = field_body.get("format")
                        if not isinstance(fmt, str):
                            continue
                        stream.fields.append(
                            StreamField(
                                name=field_name,
                                fmt=fmt,
                                unit=field_body.get("unit") if isinstance(field_body.get("unit"), str) else None,
                                elements=int(field_body.get("elements") or 1),
                                channels=int(field_body.get("channels") or 1),
                                multiply_factor=float(field_body.get("multiply_factor") or 1.0),
                                odr=field_body.get("odr") if isinstance(field_body.get("odr"), int) else None,
                                minimum=_float_or_none(field_body.get("min")),
                                maximum=_float_or_none(field_body.get("max")),
                            )
                        )

            if enabled is not None or sensor.stream_id is not None:
                sensors[name] = sensor

    return sensors, streams


def _float_or_none(value: Any) -> float | None:
    try:
        return None if value is None else float(value)
    except (TypeError, ValueError):
        return None


_FORMATS: dict[str, tuple[str, int]] = {
    "uint8_t": ("B", 1),
    "int8_t": ("b", 1),
    "uint16_t": ("<H", 2),
    "int16_t": ("<h", 2),
    "uint32_t": ("<I", 4),
    "int32_t": ("<i", 4),
    "float": ("<f", 4),
    "float_t": ("<f", 4),
}


def decode_raw_sample(data: bytes, streams: dict[int, RawStream]) -> tuple[int, str, list[tuple[str, list[float], str | None]]] | None:
    for start in (0, 2):
        if len(data) <= start:
            continue
        stream_id = data[start]
        stream = streams.get(stream_id)
        if stream is None:
            continue
        decoded = _decode_stream_fields(data[start + 1 :], stream)
        if decoded is not None:
            return stream_id, stream.component, decoded
    return None


def _decode_stream_fields(payload: bytes, stream: RawStream) -> list[tuple[str, list[float], str | None]] | None:
    offset = 0
    output: list[tuple[str, list[float], str | None]] = []
    for field in stream.fields:
        fmt_info = _FORMATS.get(field.fmt)
        if fmt_info is None:
            continue
        fmt, size = fmt_info
        count = field.elements * field.channels
        values: list[float] = []
        for _ in range(count):
            if offset + size > len(payload):
                return None
            raw_value = struct.unpack_from(fmt, payload, offset)[0]
            values.append(float(raw_value) * field.multiply_factor)
            offset += size
        output.append((field.name, values, field.unit))
    return output
