"""Exercise orderly process shutdown, including clients blocked before login."""
import os
import signal
import socket
import subprocess
import sys
import tempfile
import time


def wait_output(process, log, marker):
    deadline = time.monotonic() + 10
    while time.monotonic() < deadline:
        if marker in os.pread(log.fileno(), 1024 * 1024, 0):
            return
        if process.poll() is not None:
            raise RuntimeError("process exited before " + repr(marker))
        time.sleep(0.02)
    raise RuntimeError("timed out waiting for " + repr(marker))


def cleanup(process):
    if process is not None and process.poll() is None:
        process.kill()
        process.wait(timeout=3)


def scenario(server, client, sig, state):
    process = held_client = connection = None
    with tempfile.TemporaryFile() as log, tempfile.TemporaryFile() as client_log:
        try:
            with socket.socket() as probe:
                probe.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
                probe.bind(("127.0.0.1", 1234))
            process = subprocess.Popen([server], stdout=log, stderr=subprocess.STDOUT)
            wait_output(process, log, b"Server started. Waiting for client...")
            if state in ("tcp", "partial-handshake"):
                connection = socket.create_connection(("127.0.0.1", 1234), timeout=2)
                if state == "partial-handshake":
                    connection.sendall(b"GET / HTTP/1.1\r\nHost: localhost\r\n")
                # Let the 20ms accept loop register the connection before signaling.
                time.sleep(0.1)
            elif state == "logged-in":
                held_client = subprocess.Popen([client, "--hold"], stdout=client_log,
                                               stderr=subprocess.STDOUT)
                wait_output(held_client, client_log, b"HOLD READY")
            started = time.monotonic()
            process.send_signal(sig)
            if process.wait(timeout=8) != 0:
                raise RuntimeError("server exited unsuccessfully")
            if b"Server stopped cleanly." not in os.pread(log.fileno(), 1024 * 1024, 0):
                raise RuntimeError("shutdown completion marker missing")
            if held_client and held_client.wait(timeout=3) != 0:
                raise RuntimeError("active client did not observe disconnect")
            with socket.socket() as probe:
                probe.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
                probe.bind(("127.0.0.1", 1234))
            print("PASS", sig.name, state, "%.3fs" % (time.monotonic() - started))
        finally:
            if connection:
                connection.close()
            cleanup(held_client)
            cleanup(process)
            print(os.pread(log.fileno(), 1024 * 1024, 0).decode(errors="replace"))


if __name__ == "__main__":
    for sig in (signal.SIGINT, signal.SIGTERM):
        for state in ("idle", "tcp", "partial-handshake", "logged-in"):
            scenario(sys.argv[1], sys.argv[2], sig, state)
