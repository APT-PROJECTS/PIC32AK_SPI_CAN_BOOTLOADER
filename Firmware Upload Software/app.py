from __future__ import annotations

import struct
import threading
import time
import zlib
from collections import deque
from pathlib import Path

from flask import Flask, jsonify, render_template, request
from serial.tools import list_ports
from werkzeug.utils import secure_filename

from waveshare_can import CanFrame, WaveshareCANA


BASE_DIR = Path(__file__).resolve().parent
UPLOAD_DIR = BASE_DIR / "uploads"
UPLOAD_DIR.mkdir(exist_ok=True)

BOOT_CAN_ID_HOST = 0xB0
BOOT_CAN_ID_CLIENT = 0xB1
CAN_READ = 0x01
CAN_WRITE = 0x02
CAN_COMPARE = 0x03
CAN_SUCCESS = 0x01
CAN_FAIL = 0x02
CMD_PIC_ID = 0x01
CMD_SOFTWARE_ID = 0x02
CMD_APP_SIZE = 0x04
CMD_JUMP = 0x05
CMD_APP_ERASE = 0x06
CMD_APP_CHUNK = 0x07
CMD_APP_CRC = 0x08
SUB_PROJECT_ID = 0x01
SUB_FIRMWARE_ID = 0x02
SUB_BOOT_JUMP = 0x01
SUB_APP_JUMP = 0x02
CHUNK_SIZE = 1024
FRAME_DATA_SIZE = 4
MAX_REQUEST_ATTEMPTS = 5

app = Flask(__name__)


class UploadService:
    def __init__(self) -> None:
        self.adapter: WaveshareCANA | None = None
        self.port: str | None = None
        self.lock = threading.RLock()
        self.job_lock = threading.Lock()
        self.logs: deque[dict] = deque(maxlen=500)
        self.status = {"connected": False, "busy": False, "phase": "Idle", "progress": 0, "message": "Disconnected"}
        self.firmware: dict | None = None
        self.controller = {"pic_id": "—", "project_id": "—", "firmware_id": "—", "app_size": "—", "crc": "—"}

    def log(self, message: str, level: str = "info") -> None:
        self.logs.append({"time": time.strftime("%H:%M:%S"), "message": message, "level": level})

    def connect(self, port: str, bitrate: int) -> None:
        with self.lock:
            self.disconnect()
            self.adapter = WaveshareCANA(port, bitrate)
            self.port = port
            self.status.update(connected=True, phase="Ready", message=f"Connected to {port}")
            self.log(f"Connected to Waveshare USB-CAN on {port} at {bitrate} bit/s", "success")

    def disconnect(self) -> None:
        with self.lock:
            if self.adapter:
                self.adapter.close()
            self.adapter = None
            self.port = None
            self.status.update(connected=False, busy=False, phase="Idle", progress=0, message="Disconnected")

    def _adapter(self) -> WaveshareCANA:
        if not self.adapter:
            raise RuntimeError("Connect the USB-CAN adapter first")
        return self.adapter

    def _send(self, payload: bytes, description: str) -> None:
        self._adapter().send(BOOT_CAN_ID_HOST, payload)
        self.log(f"TX 0x{BOOT_CAN_ID_HOST:03X}: {payload.hex(' ').upper()} — {description}")

    def _wait_for(self, predicate, timeout: float = 3.0) -> CanFrame:
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            frame = self._adapter().receive(min(0.2, deadline - time.monotonic()))
            if frame is None:
                continue
            # The activity panel is reserved for the bootloader protocol only.
            if frame.can_id in (BOOT_CAN_ID_HOST, BOOT_CAN_ID_CLIENT):
                self.log(f"RX 0x{frame.can_id:03X}: {frame.data.hex(' ').upper()}")
            if frame.can_id == BOOT_CAN_ID_CLIENT and predicate(frame.data):
                return frame
        raise TimeoutError("Timed out waiting for controller CAN reply")

    def _request(self, payload: bytes, description: str, matcher, timeout: float = 3.0) -> bytes:
        for attempt in range(1, MAX_REQUEST_ATTEMPTS + 1):
            self._send(payload, f"{description} (attempt {attempt}/{MAX_REQUEST_ATTEMPTS})")
            try:
                return self._wait_for(matcher, timeout).data
            except TimeoutError:
                if attempt == MAX_REQUEST_ATTEMPTS:
                    raise TimeoutError(f"No controller response after {MAX_REQUEST_ATTEMPTS} attempts: {description}")
                self.log(f"No response for {description}; retrying ({attempt}/{MAX_REQUEST_ATTEMPTS})", "warning")

    @staticmethod
    def _reply(command: int, operation: int, subcommand: int | None = None):
        def matches(data: bytes) -> bool:
            return len(data) >= 4 and data[0] == command and data[2] == operation and (subcommand is None or data[1] == subcommand)
        return matches

    def _read_controller_info(self) -> None:
        pic_id = bytearray()
        for index in range(1, 5):
            response = self._request(bytes([CMD_PIC_ID, index, CAN_READ, 0, 0, 0, 0, 0]), f"Read PIC ID word {index}", self._reply(CMD_PIC_ID, CAN_READ, index))
            if response[3] != CAN_SUCCESS:
                raise RuntimeError("Controller rejected PIC ID read")
            pic_id.extend(response[4:8])
        self.controller["pic_id"] = pic_id.hex().upper()

        for label, subcommand in (("project_id", SUB_PROJECT_ID), ("firmware_id", SUB_FIRMWARE_ID)):
            response = self._request(bytes([CMD_SOFTWARE_ID, subcommand, CAN_READ, 0, 0, 0, 0, 0]), f"Read {label}", self._reply(CMD_SOFTWARE_ID, CAN_READ, subcommand))
            if response[3] != CAN_SUCCESS:
                raise RuntimeError(f"Controller rejected {label} read")
            self.controller[label] = f"{struct.unpack('<f', response[4:8])[0]:.2f}"

        response = self._request(bytes([CMD_APP_SIZE, 0, CAN_READ, 0, 0, 0, 0, 0]), "Read application size", self._reply(CMD_APP_SIZE, CAN_READ))
        if response[3] != CAN_SUCCESS:
            raise RuntimeError("Controller rejected app-size read")
        self.controller["app_size"] = str(int.from_bytes(response[4:8], "big"))
        self.status.update(phase="Ready", message="Controller information updated")
        self.log("Controller information read successfully", "success")

    def start_info_read(self) -> None:
        self._start_job("Reading controller", self._read_controller_info)

    def _boot_jump(self) -> None:
        response = self._request(bytes([CMD_JUMP, SUB_BOOT_JUMP, 0, 0, 0, 0, 0, 0]), "Request bootloader mode", self._reply(CMD_JUMP, 0, SUB_BOOT_JUMP), 6.0)
        if response[3] != CAN_SUCCESS:
            raise RuntimeError("Controller rejected bootloader jump")
        self.log("Controller is in bootloader mode", "success")

    def _compare_size(self, size: int) -> None:
        response = self._request(bytes([CMD_APP_SIZE, 0, CAN_COMPARE, 0]) + size.to_bytes(4, "big"), "Compare firmware size", self._reply(CMD_APP_SIZE, CAN_COMPARE))
        if response[3] != CAN_SUCCESS:
            raise RuntimeError("Firmware size exceeds the application partition")

    def _erase(self) -> None:
        response = self._request(bytes([CMD_APP_ERASE, 0, 0, 0, 0, 0, 0, 0]), "Erase application flash", self._reply(CMD_APP_ERASE, 0), 15.0)
        if response[3] != CAN_SUCCESS:
            raise RuntimeError("Application erase failed")

    def _send_firmware(self, image: bytes) -> None:
        chunk_count = (len(image) + CHUNK_SIZE - 1) // CHUNK_SIZE
        for chunk_number in range(chunk_count):
            chunk_index = (chunk_number % 255) + 1
            start = chunk_number * CHUNK_SIZE
            chunk = image[start:start + CHUNK_SIZE].ljust(CHUNK_SIZE, b"\xFF")
            for attempt in range(1, MAX_REQUEST_ATTEMPTS + 1):
                for offset in range(0, CHUNK_SIZE, FRAME_DATA_SIZE):
                    payload = bytes([CMD_APP_CHUNK, chunk_index, CAN_WRITE, 0]) + chunk[offset:offset + FRAME_DATA_SIZE]
                    self._adapter().send(BOOT_CAN_ID_HOST, payload)
                    time.sleep(0.001)
                self.log(f"TX chunk {chunk_number + 1}/{chunk_count} ({min(start + CHUNK_SIZE, len(image))}/{len(image)} bytes), attempt {attempt}/{MAX_REQUEST_ATTEMPTS}")
                try:
                    reply = self._wait_for(self._reply(CMD_APP_CHUNK, CAN_WRITE, chunk_index), 6.0).data
                except TimeoutError:
                    if attempt == MAX_REQUEST_ATTEMPTS:
                        raise TimeoutError(f"No acknowledgement for firmware chunk {chunk_number + 1} after {MAX_REQUEST_ATTEMPTS} attempts")
                    self.log(f"No acknowledgement for chunk {chunk_number + 1}; retransmitting", "warning")
                    continue
                if reply[3] != CAN_SUCCESS:
                    raise RuntimeError(f"Controller rejected firmware chunk {chunk_number + 1}")
                break
            self.status["progress"] = round((chunk_number + 1) * 100 / chunk_count)

    def _verify_crc(self, image: bytes) -> None:
        host_crc = zlib.crc32(image) & 0xFFFFFFFF
        response = self._request(bytes([CMD_APP_CRC, 0, CAN_READ, 0, 0, 0, 0, 0]), "Read application CRC", self._reply(CMD_APP_CRC, CAN_READ), 8.0)
        if response[3] != CAN_SUCCESS:
            raise RuntimeError("Controller CRC calculation failed")
        controller_crc = int.from_bytes(response[4:8], "big")
        self.controller["crc"] = f"0x{controller_crc:08X}"
        if controller_crc != host_crc:
            raise RuntimeError(f"CRC mismatch: host 0x{host_crc:08X}, controller 0x{controller_crc:08X}")
        self.log(f"CRC verified: 0x{controller_crc:08X}", "success")

    def _write_app_size(self, size: int) -> None:
        response = self._request(
            bytes([CMD_APP_SIZE, 0, CAN_WRITE, 0]) + size.to_bytes(4, "big"),
            "Write application size",
            self._reply(CMD_APP_SIZE, CAN_WRITE),
        )
        if response[3] != CAN_SUCCESS:
            raise RuntimeError("Controller failed to save application size")
        self.controller["app_size"] = str(size)
        self.log(f"Application size saved: {size:,} bytes", "success")

    def _app_jump(self) -> None:
        response = self._request(bytes([CMD_JUMP, SUB_APP_JUMP, 0, 0, 0, 0, 0, 0]), "Jump to application", self._reply(CMD_JUMP, 0, SUB_APP_JUMP), 4.0)
        if response[3] != CAN_SUCCESS:
            raise RuntimeError("Controller rejected application jump")

    def _upload(self) -> None:
        if not self.firmware:
            raise RuntimeError("Upload a .bin file first")
        image = self.firmware["path"].read_bytes()
        self.status.update(phase="Reading controller information", message="Reading PIC ID and software IDs")
        self._read_controller_info()
        self.status.update(phase="Requesting bootloader", message="Switching controller to bootloader mode")
        self._boot_jump()
        self._compare_size(len(image))
        self._erase()
        self.status.update(phase="Uploading chunks", progress=0)
        self._send_firmware(image)
        self.status["phase"] = "Validating CRC"
        self._verify_crc(image)
        self.status["phase"] = "Saving application size"
        self._write_app_size(len(image))
        self.status["phase"] = "Starting application"
        self._app_jump()
        self.status.update(progress=100, phase="Complete", message="Firmware update completed")
        self.log("Firmware update completed successfully", "success")

    def start_upload(self) -> None:
        self._start_job("Preparing update", self._upload)

    def _start_job(self, phase: str, task) -> None:
        if not self.status["connected"]:
            raise RuntimeError("Connect the USB-CAN adapter first")
        if not self.job_lock.acquire(blocking=False):
            raise RuntimeError("Another operation is already running")
        self.status.update(busy=True, phase=phase, progress=0, message=phase)

        def worker() -> None:
            try:
                with self.lock:
                    task()
            except Exception as error:  # Display hardware/protocol errors to the operator.
                self.status.update(phase="Error", message=str(error))
                self.log(str(error), "error")
            finally:
                self.status["busy"] = False
                self.job_lock.release()

        threading.Thread(target=worker, daemon=True).start()


service = UploadService()


@app.get("/")
def index():
    return render_template("index.html")


@app.get("/api/ports")
def ports():
    return jsonify([{"device": port.device, "description": port.description} for port in list_ports.comports()])


@app.post("/api/connect")
def connect():
    data = request.get_json(force=True)
    service.connect(data["port"], int(data.get("bitrate", 500000)))
    return jsonify({"ok": True})


@app.post("/api/disconnect")
def disconnect():
    service.disconnect()
    return jsonify({"ok": True})


@app.post("/api/firmware")
def upload_file():
    file = request.files.get("firmware")
    if not file or not file.filename.lower().endswith(".bin"):
        return jsonify({"error": "Select a .bin firmware file"}), 400
    filename = secure_filename(file.filename)
    path = UPLOAD_DIR / filename
    file.save(path)
    size = path.stat().st_size
    if size == 0:
        path.unlink(missing_ok=True)
        return jsonify({"error": "Firmware file is empty"}), 400
    service.firmware = {"name": filename, "path": path, "size": size, "crc": zlib.crc32(path.read_bytes()) & 0xFFFFFFFF}
    service.log(f"Firmware loaded: {filename} ({size:,} bytes)", "success")
    return jsonify({"ok": True, "name": filename, "size": size, "crc": f"0x{service.firmware['crc']:08X}"})


@app.post("/api/controller/read")
def read_controller():
    service.start_info_read()
    return jsonify({"ok": True})


@app.post("/api/update")
def update():
    service.start_upload()
    return jsonify({"ok": True})


@app.get("/api/status")
def status():
    firmware = None if not service.firmware else {key: value for key, value in service.firmware.items() if key != "path"}
    return jsonify({"status": service.status, "firmware": firmware, "controller": service.controller, "logs": list(service.logs)})


@app.errorhandler(Exception)
def handle_error(error):
    return jsonify({"error": str(error)}), 400


if __name__ == "__main__":
    app.run(host="127.0.0.1", port=5000, debug=True)
