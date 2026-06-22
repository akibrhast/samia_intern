from __future__ import annotations

import asyncio
from datetime import datetime
from typing import Any

from bleak import BleakClient, BleakScanner
from bleak.backends.device import BLEDevice

from .pnpl import (
    RawStream,
    SensorComponent,
    command_get_status,
    command_log_controller_status,
    command_sensor_enable,
    command_set_time,
    command_start_ble,
    command_stop,
    decode_raw_sample,
    parse_components,
    parse_json_payload,
)
from .protocol import PNPL_UUID, RAW_PNPL_UUID, DataTransporter, split_write_frames


class STBleTerminalClient:
    def __init__(self, device: BLEDevice | str, mtu: int = 20) -> None:
        self.device = device
        self.mtu = mtu
        self.client: BleakClient | None = None
        self.transporter = DataTransporter()
        self.sensors: dict[str, SensorComponent] = {}
        self.streams: dict[int, RawStream] = {}
        self.raw_printing = True
        self.connected_event = asyncio.Event()
        self._reconnect_task: asyncio.Task[None] | None = None
        self._wanted_streaming = False

    async def connect(self) -> None:
        self.client = BleakClient(self.device, disconnected_callback=self._on_disconnect)
        await self.client.connect()
        self.connected_event.set()
        await self._subscribe()
        await self.request_status()
        print(f"connected: {self._device_label()}")

    async def disconnect(self) -> None:
        self._wanted_streaming = False
        if self.client and self.client.is_connected:
            await self.client.disconnect()

    async def reconnect(self) -> None:
        self.connected_event.clear()
        while True:
            try:
                self.client = BleakClient(self.device, disconnected_callback=self._on_disconnect)
                await self.client.connect()
                self.connected_event.set()
                await self._subscribe()
                await self.request_status()
                print("reconnected")
                if self._wanted_streaming:
                    await self.start_streaming()
                return
            except Exception as exc:
                print(f"reconnect failed: {exc}; retrying in 2s")
                await asyncio.sleep(2)

    async def request_status(self) -> None:
        await self.send_json(command_get_status())
        await asyncio.sleep(0.2)
        await self.send_json(command_log_controller_status())

    async def set_sensor_enabled(self, sensor_name: str, enabled: bool) -> None:
        await self.send_json(command_sensor_enable(sensor_name, enabled))
        await asyncio.sleep(0.3)
        await self.request_status()

    async def start_streaming(self) -> None:
        self._wanted_streaming = True
        await self.send_json(command_set_time(datetime.now().strftime("%Y%m%d_%H_%M_%S")))
        await asyncio.sleep(0.2)
        await self.send_json(command_start_ble())
        print("start sent with interface:2")

    async def stop_streaming(self) -> None:
        self._wanted_streaming = False
        await self.send_json(command_stop())
        print("stop sent")

    async def send_json(self, json_text: str) -> None:
        client = self._require_client()
        framed = self.transporter.encapsulate_json(json_text)
        for frame in split_write_frames(framed, self.mtu):
            await client.write_gatt_char(PNPL_UUID, frame, response=False)
            await asyncio.sleep(0.1)
        print(f"> {json_text}")

    async def _subscribe(self) -> None:
        client = self._require_client()
        services = client.services
        uuids = {char.uuid.lower() for service in services for char in service.characteristics}
        if PNPL_UUID not in uuids:
            raise RuntimeError(f"PnPL characteristic not found: {PNPL_UUID}")
        await client.start_notify(PNPL_UUID, self._on_pnpl_notify)
        if RAW_PNPL_UUID in uuids:
            await client.start_notify(RAW_PNPL_UUID, self._on_raw_notify)
        else:
            print(f"warning: Raw PnPL characteristic not found: {RAW_PNPL_UUID}")

    def _on_pnpl_notify(self, _sender: Any, data: bytearray) -> None:
        payload = self.transporter.decapsulate(data)
        if payload is None:
            return
        parsed = parse_json_payload(payload)
        if parsed is None:
            print(f"< pnpl undecoded: {payload.hex()}")
            return
        sensors, streams = parse_components(parsed)
        if sensors:
            self.sensors.update(sensors)
        if streams:
            self.streams.update(streams)
        print(f"< pnpl {parsed}")

    def _on_raw_notify(self, _sender: Any, data: bytearray) -> None:
        if not self.raw_printing:
            return
        decoded = decode_raw_sample(bytes(data), self.streams)
        if decoded is None:
            print(f"< raw undecoded {bytes(data).hex()}")
            return
        stream_id, component, fields = decoded
        parts = []
        for name, values, unit in fields:
            preview = ", ".join(f"{value:.5g}" for value in values[:8])
            if len(values) > 8:
                preview += f", ... ({len(values)} values)"
            parts.append(f"{name}=[{preview}]{unit or ''}")
        print(f"< raw stream={stream_id} component={component} " + " ".join(parts))

    def _on_disconnect(self, _client: BleakClient) -> None:
        self.connected_event.clear()
        print("disconnected")
        if self._reconnect_task is None or self._reconnect_task.done():
            self._reconnect_task = asyncio.create_task(self.reconnect())

    def _require_client(self) -> BleakClient:
        if self.client is None or not self.client.is_connected:
            raise RuntimeError("not connected")
        return self.client

    def _device_label(self) -> str:
        name = getattr(self.device, "name", None) or "unknown"
        address = getattr(self.device, "address", None) or str(self.device)
        return f"{name} [{address}]"


async def scan_devices(timeout: float = 6.0) -> list[BLEDevice]:
    devices = await BleakScanner.discover(timeout=timeout)
    return sorted(devices, key=lambda d: (d.name or "", d.address))
