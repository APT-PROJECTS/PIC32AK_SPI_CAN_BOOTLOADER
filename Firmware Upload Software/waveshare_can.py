"""Waveshare USB-CAN-A serial framing used by the offline firmware uploader."""

from __future__ import annotations

import time
from dataclasses import dataclass

import serial


SERIAL_BAUDRATE = 2_000_000
CAN_BITRATE_CODES = {
    1_000_000: 0x01,
    800_000: 0x02,
    500_000: 0x03,
    400_000: 0x04,
    250_000: 0x05,
    200_000: 0x06,
    125_000: 0x07,
    100_000: 0x08,
    50_000: 0x09,
    20_000: 0x0A,
    10_000: 0x0B,
    5_000: 0x0C,
}


@dataclass(frozen=True)
class CanFrame:
    can_id: int
    data: bytes
    extended: bool = False
    remote: bool = False


def _config_checksum(frame: bytes) -> int:
    return sum(frame[2:19]) & 0xFF


def build_config_frame(can_bitrate: int) -> bytes:
    if can_bitrate not in CAN_BITRATE_CODES:
        raise ValueError(f"Unsupported CAN bitrate: {can_bitrate}")
    frame = bytearray(20)
    frame[0:3] = b"\xAA\x55\x12"
    frame[3] = CAN_BITRATE_CODES[can_bitrate]
    frame[4] = 0x01
    frame[19] = _config_checksum(frame)
    return bytes(frame)


def build_variable_frame(can_id: int, data: bytes, extended: bool = False) -> bytes:
    if not 0 <= len(data) <= 8:
        raise ValueError("CAN payload must be 0..8 bytes")
    if extended:
        if not 0 <= can_id <= 0x1FFFFFFF:
            raise ValueError("Invalid extended CAN ID")
        return b"\xAA" + bytes([0xE0 | len(data)]) + can_id.to_bytes(4, "little") + data + b"\x55"
    if not 0 <= can_id <= 0x7FF:
        raise ValueError("Invalid standard CAN ID")
    return b"\xAA" + bytes([0xC0 | len(data)]) + can_id.to_bytes(2, "little") + data + b"\x55"


def _read_exact(ser: serial.Serial, count: int, deadline: float) -> bytes | None:
    value = bytearray()
    while len(value) < count and time.monotonic() < deadline:
        chunk = ser.read(count - len(value))
        if chunk:
            value.extend(chunk)
    return bytes(value) if len(value) == count else None


def read_variable_frame(ser: serial.Serial, timeout: float) -> CanFrame | None:
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        if ser.read(1) != b"\xAA":
            continue
        type_data = _read_exact(ser, 1, deadline)
        if not type_data:
            return None
        frame_type = type_data[0]
        length = frame_type & 0x0F
        extended = bool(frame_type & 0x20)
        remote = bool(frame_type & 0x10)
        if length > 8:
            continue
        id_length = 4 if extended else 2
        value = _read_exact(ser, id_length + length + 1, deadline)
        if not value or value[-1] != 0x55:
            continue
        return CanFrame(int.from_bytes(value[:id_length], "little"), value[id_length:-1], extended, remote)
    return None


class WaveshareCANA:
    def __init__(self, port: str, can_bitrate: int) -> None:
        self.ser = serial.Serial(port, SERIAL_BAUDRATE, timeout=0.02)
        self.ser.reset_input_buffer()
        self.ser.reset_output_buffer()
        self.ser.write(build_config_frame(can_bitrate))
        self.ser.flush()
        time.sleep(0.2)
        self.ser.reset_input_buffer()

    def send(self, can_id: int, data: bytes) -> None:
        self.ser.write(build_variable_frame(can_id, data))
        self.ser.flush()

    def receive(self, timeout: float) -> CanFrame | None:
        return read_variable_frame(self.ser, timeout)

    def close(self) -> None:
        if self.ser.is_open:
            self.ser.close()
