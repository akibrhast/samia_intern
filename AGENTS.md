# Repository Working Notes

## Current Objective

This repository is being narrowed around the `STEVAL-STWINKT1B` / `STM32L4R9ZI-STWIN` FP-SNS-DATALOG2 target.

The active workflow is:

1. Build and flash the STWIN DATALOG2 firmware.
2. Power-cycle the board.
3. Confirm the board advertises over BLE as `HSD2v33`.
4. Connect from the ST BLE Sensor mobile app or from the local Python BLE terminal client.
5. Use BLE as the primary development/test path for sensor control and streaming.

The preferred host workflow is Bluetooth-first. USB/serial can still be useful for debugging or comparison, but the main goal is to avoid repeatedly connecting and reconnecting serial while developing.

## Implemented Firmware Changes

The board firmware changes are in:

- `Projects/STM32L4R9ZI-STWIN/Applications/DATALOG2/Core/Src/UtilTask.c`
- `Projects/STM32L4R9ZI-STWIN/Applications/DATALOG2/Core/Src/DatalogAppTask.c`

Current behavior:

- The idle LED heartbeat toggles every 4 seconds.
- BLE start/stop support was added for `LOG_CTRL_MODE_BLE`.
- If a start request comes in through the SD path and no SD card is mounted, the firmware can fall back to BLE streaming.
- BLE start disables USB/FileX, starts BLE streaming, updates the streaming status, and posts the power-management event.
- BLE stop stops BLE streaming, deallocates streaming resources, resets the log controller interface, re-enables USB/FileX, and stops the timestamp.

Known mobile-app behavior:

- The ST BLE Sensor app can discover and connect to the board.
- The app may show a missing SD card message because its HSD screen can block start before the firmware fallback is reached.
- The firmware side is prepared for BLE streaming, but the stock app path is not always the best way to exercise it.

## Python BLE Terminal Client

A local BLE-first terminal client was added under:

- `Projects/STM32L4R9ZI-STWIN/Applications/DATALOG2/STM32CubeIDE/ble_terminal_client`

It is intended to provide the laptop-side workflow:

- Scan/select BLE devices.
- Connect/reconnect.
- Query PnPL status.
- List sensors/streams.
- Enable/disable sensors.
- Start/stop BLE streaming using interface `2`.
- Print terminal output from BLE notifications.

This client remains important because ST's official Python SDK does not appear to provide a laptop BLE transport implementation.

## ST `stdatalog-pysdk` Findings

The official ST repo `https://github.com/STMicroelectronics/stdatalog-pysdk` is useful reference material, but it does not replace the BLE client for this project.

Useful pieces:

- PnPL command shapes and command-builder patterns.
- USB/serial examples for comparison testing.
- Sensor metadata and decoding architecture.
- Dataset/log parsing and export utilities.
- GUI/reference application structure for future host tooling.

Important limitation:

- The SDK supports USB/native HSD and serial datalog paths.
- It does not appear to include BLE scan/connect/GATT notification handling.
- It has references to BLE metadata and interface `2`, but not the host-side Bluetooth transport needed for laptop BLE streaming.

Decision:

- Keep the custom Python BLE client as the primary host application path.
- Use `stdatalog-pysdk` as an official protocol and parsing reference as the BLE client matures.
- Use USB/serial SDK examples only as a fallback/debug comparison path.

## Repository Cleanup Already Done

This repo was cleaned to focus on the STWIN DATALOG2 target.

Removed unrelated board examples:

- `Projects/B-U585I-IOT02A`
- `Projects/NUCLEO-H563ZI`
- `Projects/NUCLEO-H7A3ZI-Q`
- `Projects/NUCLEO-U575ZI-Q`
- `Projects/STM32H7B3RI-AFCI1`
- `Projects/STM32U585AI-STWIN.box`
- `Projects/STM32U585AI-SensorTile.boxPro`

Removed unrelated utility/example folders:

- `Utilities/STWIN.box_config_examples`
- `Utilities/STWIN.box_acquisition_examples`
- `Utilities/Sensortile.boxPRO_config_examples`
- `Utilities/Sensortile.boxPRO_acquisition_examples`
- `Utilities/BoardReset`
- `Utilities/WiFi_module_upgrade`
- `Utilities/ISPU_config_examples`

`.gitignore` was updated to ignore common local/generated files such as STM32CubeIDE build outputs, virtual environments, Python caches, pytest cache, egg-info folders, and `.DS_Store`.

## Lessons Learned

- BLE advertisement and connection are working; the board appears as `HSD2v33`.
- The board reports firmware as `FP-SNS-DATALOG2_Datalog2 v3.3.0`.
- A missing SD card does not necessarily mean BLE streaming is impossible; it can be an app-side workflow limitation.
- For live sensor data over BLE, the host needs both PnPL control and RawPnPL/BLE notification decoding.
- The firmware's logging interface matters:
  - `1` is USB.
  - `2` is BLE.
  - `3` is serial in ST's Python SDK examples.
- ST's SDK is strong for USB/serial workflows, but the Bluetooth laptop workflow still needs custom code.

## Current Plan

Near-term:

- Keep BLE as the primary development workflow.
- Validate BLE start/stop and raw notification flow from the Python terminal client.
- Decode raw BLE payloads into useful terminal sensor values.
- Keep the terminal UI simple before adding charts or a full desktop GUI.

Next steps:

- Reuse `stdatalog-pysdk` ideas for PnPL command generation and sensor metadata parsing.
- Add stronger reconnect/session recovery in the BLE client.
- Add clearer device and sensor selection flows.
- Add structured output modes, for example JSON lines, for easier future UI integration.
- Compare behavior against USB/serial SDK examples when BLE output appears suspicious.

Longer-term:

- Build a richer desktop UI on top of the BLE client once terminal streaming is reliable.
- Add charts, sensor panels, recording controls, and export tools.
- Consider selectively depending on ST SDK packages for parsing/post-processing instead of copying that logic.

## Command-Line Firmware Build

The STM32CubeIDE GUI does not need to be opened for normal firmware builds.
This project is a CubeIDE managed-build project, so the terminal build still uses the installed STM32CubeIDE application and toolchain in headless mode.

Clean Debug build:

```sh
"/Applications/STM32CubeIDE.app/Contents/MacOS/STM32CubeIDE" \
  -nosplash \
  -application org.eclipse.cdt.managedbuilder.core.headlessbuild \
  -data /tmp/stm32cubeide-workspace \
  -import /Users/arahman/Documents/fp-sns-datalog2/Projects/STM32L4R9ZI-STWIN/Applications/DATALOG2/STM32CubeIDE \
  -cleanBuild DATALOG2-STWINKT1B/Debug
```

Faster incremental Debug build:

```sh
"/Applications/STM32CubeIDE.app/Contents/MacOS/STM32CubeIDE" \
  -nosplash \
  -application org.eclipse.cdt.managedbuilder.core.headlessbuild \
  -data /tmp/stm32cubeide-workspace \
  -import /Users/arahman/Documents/fp-sns-datalog2/Projects/STM32L4R9ZI-STWIN/Applications/DATALOG2/STM32CubeIDE \
  -build DATALOG2-STWINKT1B/Debug
```

Use `-cleanBuild` when a full rebuild is needed. Use `-build` for normal iteration after code edits.

Expected outputs:

- `Projects/STM32L4R9ZI-STWIN/Applications/DATALOG2/STM32CubeIDE/Debug/DATALOG2-STWINKT1B.elf`
- `Projects/STM32L4R9ZI-STWIN/Applications/DATALOG2/STM32CubeIDE/Debug/DATALOG2-STWINKT1B.bin`

The last verified headless Debug build completed with `0 errors, 0 warnings` and produced:

```text
text 399428, data 3752, bss 645632, dec 1048812, hex 1000ec
```

## Working Guidance For Future Agents

- Prefer `rg`/`rg --files` for searching this repo.
- Do not reintroduce unrelated board projects unless the user explicitly asks.
- Treat the repository as focused on `Projects/STM32L4R9ZI-STWIN/Applications/DATALOG2`.
- Do not assume the ST BLE Sensor app is the final validation tool; it is useful but can impose app-side behavior that differs from the intended laptop BLE workflow.
- Before changing firmware behavior, check both the C firmware path and the Python BLE client expectations.
- Keep firmware changes scoped and build-test when practical.
- Keep Python BLE client changes terminal-first until BLE streaming is proven reliable.
