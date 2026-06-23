from __future__ import annotations

import argparse
import asyncio

from .client import STBleTerminalClient, scan_devices


async def async_input(prompt: str) -> str:
    return await asyncio.to_thread(input, prompt)


async def choose_device(timeout: float) -> object:
    print(f"scanning for {timeout:g}s...")
    devices = await scan_devices(timeout)
    if not devices:
        raise SystemExit("no BLE devices found")

    for index, device in enumerate(devices):
        print(f"[{index}] {device.name or '(no name)'}  {device.address}")

    while True:
        raw = await async_input("select device index: ")
        try:
            return devices[int(raw)]
        except (ValueError, IndexError):
            print("invalid selection")


async def command_loop(client: STBleTerminalClient) -> None:
    print("type 'help' for commands")
    while True:
        try:
            line = (await async_input("stble> ")).strip()
        except (EOFError, KeyboardInterrupt):
            line = "quit"
        if not line:
            continue

        parts = line.split()
        cmd = parts[0].lower()
        args = parts[1:]

        try:
            if cmd in {"quit", "exit"}:
                await client.stop_streaming()
                await client.disconnect()
                return
            if cmd == "help":
                print_help()
            elif cmd == "status":
                await client.request_status()
            elif cmd == "sensors":
                print_sensors(client)
            elif cmd == "streams":
                print_streams(client)
            elif cmd == "enable" and len(args) == 1:
                await client.set_sensor_enabled(args[0], True)
            elif cmd == "disable" and len(args) == 1:
                await client.set_sensor_enabled(args[0], False)
            elif cmd == "start":
                await client.start_streaming()
            elif cmd == "stop":
                await client.stop_streaming()
            elif cmd == "raw" and len(args) == 1 and args[0] in {"on", "off"}:
                client.raw_printing = args[0] == "on"
                print(f"raw printing {'enabled' if client.raw_printing else 'disabled'}")
            elif cmd == "reconnect":
                await client.reconnect()
            else:
                print("unknown command or bad arguments")
        except Exception as exc:
            print(f"error: {exc}")


def print_help() -> None:
    print(
        "\n".join(
            [
                "help                         show commands",
                "status                       refresh PnPL status",
                "sensors                      list sensor components",
                "enable <sensor_name>         send {sensor:{enable:true}}",
                "disable <sensor_name>        send {sensor:{enable:false}}",
                "streams                      list discovered raw BLE streams",
                "start                        set time and start BLE stream interface:2",
                "stop                         stop logging/streaming",
                "raw on|off                   show/hide raw decoded output",
                "reconnect                    reconnect manually",
                "quit                         stop and exit",
            ]
        )
    )


def print_sensors(client: STBleTerminalClient) -> None:
    if not client.sensors:
        print("no sensors parsed yet; run 'status'")
        return
    for name in sorted(client.sensors):
        sensor = client.sensors[name]
        status = "enabled" if sensor.enabled else "disabled" if sensor.enabled is False else "unknown"
        stream = f" stream={sensor.stream_id}" if sensor.stream_id is not None else ""
        print(f"{name:28} {status}{stream}")


def print_streams(client: STBleTerminalClient) -> None:
    if not client.streams:
        print("no streams parsed yet; run 'status'")
        return
    for stream_id in sorted(client.streams):
        stream = client.streams[stream_id]
        fields = ", ".join(
            f"{field.name}:{field.fmt}x{field.elements * field.channels}"
            for field in stream.fields
        )
        print(f"{stream_id:3} {stream.component:28} {fields}")


async def run() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--address", help="connect directly to a BLE address/UUID")
    parser.add_argument("--scan-timeout", type=float, default=6.0)
    parser.add_argument("--mtu", type=int, default=20, help="ST DataTransporter write chunk size")
    parser.add_argument("--dump-services", action="store_true", help="connect and print discovered GATT services")
    args = parser.parse_args()

    if args.address:
        device = args.address
    else:
        device = await choose_device(args.scan_timeout)

    client = STBleTerminalClient(device, mtu=args.mtu)
    if args.dump_services:
        client.client = client._new_client()
        await client.client.connect()
        try:
            await client.dump_services()
        finally:
            await client.client.disconnect()
        return

    await client.connect()
    await command_loop(client)


def main() -> None:
    asyncio.run(run())


if __name__ == "__main__":
    main()
