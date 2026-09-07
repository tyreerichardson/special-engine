"""Own the test server process; never connect the test to an existing server."""
import os
import pathlib
import socket
import signal
import subprocess
import sys
import tempfile
import time


def main():
    server, client = map(str, map(pathlib.Path, sys.argv[1:3]))
    process = None
    with tempfile.TemporaryFile(mode="w+b") as log:
        try:
            # Do not stop another server or silently test against it.
            with socket.socket() as probe:
                probe.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
                probe.bind(("127.0.0.1", 1234))
            process = subprocess.Popen([server], stdout=log, stderr=subprocess.STDOUT)
            deadline = time.monotonic() + 10
            while True:
                if process.poll() is not None:
                    raise RuntimeError("test server exited during startup")
                output = os.pread(log.fileno(), 1024 * 1024, 0).decode(errors="replace")
                if "Server started. Waiting for client..." in output:
                    if "Persistence disabled:" not in output:
                        raise RuntimeError("smoke test requires a persistence-disabled server")
                    break
                if time.monotonic() >= deadline:
                    raise RuntimeError("test server startup timed out")
                time.sleep(0.05)
            subprocess.run([client], check=True, timeout=15)
            if process.poll() is not None:
                raise RuntimeError("server exited after client activity")
            process.send_signal(signal.SIGTERM)
            if process.wait(timeout=8) != 0:
                raise RuntimeError("server did not exit successfully on SIGTERM")
            output = os.pread(log.fileno(), 1024 * 1024, 0).decode(errors="replace")
            if "Server stopped cleanly." not in output:
                raise RuntimeError("missing shutdown completion marker")
            with socket.socket() as released:
                released.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
                released.bind(("127.0.0.1", 1234))
            return 0
        except Exception as error:
            print("Smoke test failed:", error, file=sys.stderr)
            return 1
        finally:
            if process is not None and process.poll() is None:
                # Cleanup only: this does not assert graceful application shutdown.
                process.terminate()
                try:
                    process.wait(timeout=3)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.wait(timeout=3)
            print(os.pread(log.fileno(), 1024 * 1024, 0).decode(errors="replace"))


if __name__ == "__main__":
    sys.exit(main())
