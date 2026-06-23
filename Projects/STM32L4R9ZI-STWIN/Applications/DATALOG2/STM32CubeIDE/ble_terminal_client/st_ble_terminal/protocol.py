from __future__ import annotations

from dataclasses import dataclass


PNPL_UUID = "0000001b-0002-11e1-ac36-0002a5d5c51b"
RAW_PNPL_UUID = "00000023-0002-11e1-ac36-0002a5d5c51b"
FEATURES_SERVICE_UUID = "00000000-0001-11e1-9ab4-0002a5d5c51b"


@dataclass
class TransportConfig:
    mtu: int = 20
    start_packet: int = 0x00
    start_long_packet: int = 0x10
    start_end_packet: int = 0x20
    middle_packet: int = 0x40
    end_packet: int = 0x80


class DataTransporter:
    """Port of STBlueSDK DataTransporter framing for JSON PnPL commands."""

    def __init__(self, config: TransportConfig | None = None) -> None:
        self.config = config or TransportConfig()
        self._buffer = bytearray()

    def decapsulate(self, data: bytes | bytearray) -> bytes | None:
        if not data:
            return None

        head = data[0]
        payload = data[1:]
        if head == self.config.start_packet:
            self._buffer.clear()
            self._buffer.extend(payload)
            return None
        if head == self.config.start_end_packet:
            self._buffer.clear()
            self._buffer.extend(payload)
            return bytes(self._buffer)
        if head == self.config.middle_packet:
            self._buffer.extend(payload)
            return None
        if head == self.config.end_packet:
            self._buffer.extend(payload)
            return bytes(self._buffer)
        return None

    def encapsulate_json(self, json_text: str) -> bytes:
        command = json_text.encode("utf-8")
        mtu = self.config.mtu
        count = 0
        result = bytearray()
        coded_len = len(command)
        is_short = coded_len < (1 << 16) - 1
        length_bytes = coded_len.to_bytes(2 if is_short else 4, "big")
        head = self.config.start_packet

        while count < coded_len:
            size = min(mtu - 1, coded_len - count)
            if coded_len - count <= mtu - 1:
                if count == 0:
                    if coded_len - count <= mtu - 3:
                        head = self.config.start_end_packet
                    else:
                        head = self.config.start_packet if is_short else self.config.start_end_packet
                else:
                    head = self.config.end_packet

            if head == self.config.start_long_packet:
                to = mtu - 5
                result.append(head)
                result.extend(length_bytes)
                result.extend(command[0:to])
                size = mtu - 5
                head = self.config.middle_packet
            elif head == self.config.start_packet:
                to = mtu - 3
                result.append(head)
                result.extend(length_bytes)
                result.extend(command[0:to])
                size = mtu - 3
                head = self.config.middle_packet
            elif head == self.config.start_end_packet:
                result.append(head)
                result.extend(length_bytes)
                result.extend(command)
                size = coded_len
                head = self.config.start_packet
            elif head == self.config.middle_packet:
                to = count + mtu - 1
                result.append(head)
                result.extend(command[count:to])
            elif head == self.config.end_packet:
                result.append(head)
                result.extend(command[count:])
                head = self.config.start_packet

            count += size

        return bytes(result)


def split_write_frames(data: bytes, mtu: int) -> list[bytes]:
    return [data[index : index + mtu] for index in range(0, len(data), mtu)]
