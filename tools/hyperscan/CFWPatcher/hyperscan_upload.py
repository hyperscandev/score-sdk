#!/usr/bin/env python3
import argparse
import socket
import struct
import time
from pathlib import Path


def upload_wifi(host, file, port=7777, chunk=256, delay=0.002):
    data = Path(file).read_bytes()
    packet_len = 4 + len(data)

    print(f"[+] Connecting to {host}:{port}")
    with socket.create_connection((host, port), timeout=10) as s:
        s.settimeout(15)
        s.sendall(struct.pack("<I", len(data)))

        sent = 4
        for i in range(0, len(data), chunk):
            part = data[i:i + chunk]
            s.sendall(part)
            sent += len(part)
            print(f"\r[+] Sent {sent}/{packet_len}", end="")
            if delay:
                time.sleep(delay)

        print()
        try:
            print("[ESP]", s.recv(128).decode(errors="replace").strip())
        except socket.timeout:
            print("[!] No ESP response before timeout")

    print("[+] Done")


def upload_serial(port, file, baud=115200, chunk=256, delay=0.002):
    import serial

    data = Path(file).read_bytes()
    packet_len = 4 + len(data)

    ser = serial.Serial(
        port,
        baudrate=baud,
        bytesize=8,
        parity=serial.PARITY_EVEN,
        stopbits=1,
        timeout=1,
        write_timeout=10,
    )

    try:
        time.sleep(0.2)
        ser.reset_input_buffer()
        ser.write(struct.pack("<I", len(data)))
        ser.flush()

        sent = 4
        for i in range(0, len(data), chunk):
            part = data[i:i + chunk]
            ser.write(part)
            ser.flush()
            sent += len(part)
            print(f"\r[+] Sent {sent}/{packet_len}", end="")
            if delay:
                time.sleep(delay)

        print("\n[+] Done")
    finally:
        ser.close()


def main():
    p = argparse.ArgumentParser(description="HyperScan recovery UART uploader")
    sub = p.add_subparsers(dest="mode", required=True)

    w = sub.add_parser("wifi", help="Upload through ESP32 raw TCP upload server")
    w.add_argument("host", help="ESP32 AP IP or router STA IP")
    w.add_argument("file")
    w.add_argument("--port", type=int, default=7777)
    w.add_argument("--chunk", type=int, default=256)
    w.add_argument("--delay", type=float, default=0.002)

    s = sub.add_parser("serial", help="Upload directly to target UART with a USB-UART adapter")
    s.add_argument("port")
    s.add_argument("file")
    s.add_argument("--baud", type=int, default=115200)
    s.add_argument("--chunk", type=int, default=256)
    s.add_argument("--delay", type=float, default=0.002)

    a = p.parse_args()

    if a.mode == "wifi":
        upload_wifi(a.host, a.file, a.port, a.chunk, a.delay)
    else:
        upload_serial(a.port, a.file, a.baud, a.chunk, a.delay)


if __name__ == "__main__":
    main()
