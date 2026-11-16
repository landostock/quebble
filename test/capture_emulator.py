"""Capture native emulator frames through its monitor, without changing game state."""
import argparse
import json
import os
from pathlib import Path
import socket
import subprocess
import time

parser = argparse.ArgumentParser()
parser.add_argument("output", type=Path)
parser.add_argument("--seconds", type=float, default=35)
parser.add_argument("--fps", type=int, default=30)
parser.add_argument("--start", action="store_true")
args = parser.parse_args()
args.output.mkdir(parents=True, exist_ok=True)
info = json.loads((Path(os.environ["TMPDIR"]) / "pb-emulator.json").read_text())
port = info["emery"]["4.33.1"]["qemu"]["monitor"]
with socket.create_connection(("127.0.0.1", port)) as monitor:
    monitor.settimeout(3)
    def response():
        data = b""
        while b"(qemu)" not in data:
            chunk = monitor.recv(8192)
            if not chunk:
                raise RuntimeError("Emulator monitor disconnected")
            data += chunk
        return data
    response()
    if args.start:
        subprocess.run(["pebble", "emu-button", "--emulator", "emery", "click", "select"], check=True)
    start = time.monotonic()
    count = round(args.seconds * args.fps)
    for frame in range(count):
        time.sleep(max(0, start + frame / args.fps - time.monotonic()))
        path = (args.output / ("frame_%04d.ppm" % frame)).resolve()
        monitor.sendall(("screendump %s\n" % json.dumps(str(path))).encode())
        data = response()
        if b"Error" in data:
            raise RuntimeError(data.decode(errors="replace"))
print("Captured %d frames in %.2fs" % (count, time.monotonic() - start))
