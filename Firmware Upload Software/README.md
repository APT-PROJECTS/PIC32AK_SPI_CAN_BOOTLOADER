# CAN Firmware Uploader

Offline Flask tool for uploading a PIC32 application `.bin` through a Waveshare USB-CAN-A converter.

## Start

```powershell
cd "Firmware Upload Software"
python -m venv .venv
.\.venv\Scripts\Activate.ps1
pip install -r requirements.txt
python app.py
```

Open `http://127.0.0.1:5000` in a browser.

## Upload sequence

1. Connect to the Waveshare COM port at 2 Mbaud serial / selected CAN bitrate.
2. Select a `.bin` application image.
3. Optionally read PIC ID, Project ID, Firmware ID and stored application size.
4. Start update.

The tool performs: bootloader jump, application-size compare, erase, 1 KiB chunk transfer with one acknowledgement per chunk, CRC readback/host comparison, application-size write to EEPROM, and application jump.

The size comparison intentionally occurs **after** bootloader jump because the bootloader keeps the approved size in RAM and a controller reset clears it. Project ID and Firmware ID writes are intentionally skipped.
