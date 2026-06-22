# ST BLE Terminal Client

Terminal client for ST FP-SNS-DATALOG2 boards, based on the ST BLE Sensor iOS app protocol.

It can:

- scan and select a BLE device
- connect and reconnect
- subscribe to PnPL and Raw PnPL characteristics
- list detected PnPL sensor components
- toggle sensor `enable`
- start/stop BLE live streaming with `interface:2`
- print decoded raw stream values in the terminal

## Install

```sh
cd ble_terminal_client
python3 -m venv .venv
source .venv/bin/activate
pip install -e .
```

On macOS, allow Terminal/iTerm Bluetooth permission if prompted.

## Run

```sh
st-ble-terminal
```

Useful commands inside the app:

```text
help
status
sensors
enable <sensor_name>
disable <sensor_name>
start
stop
streams
raw on
raw off
reconnect
quit
```

For your board, expected advertising name is usually `HSD2v33`.

## Notes

The stock ST BLE Sensor HSD screen sends `interface:0` for SD logging and blocks start if `sd_mounted` is false. This client sends `interface:2` so the firmware BLE stream path is used directly.

