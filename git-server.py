#!/usr/bin/env python3

from http.server import HTTPServer, BaseHTTPRequestHandler
import json
import time

PORT = 8000

# Reduce to speed up each iteration, at the cost of making it flakier
MAXDELAY = 15000 # us

INFO = b"000dversion 2000bls-refs004dfetch=filter ref-in-want sideband-all packfile-uris wait-for-done shallow0011server-option000esession-id000fobject-info0017agent=JGit/4-google0000"

with open("timing.json", "rb") as f:
    TIMING = json.load(f)

with open("refs.txt", "rb") as f:
    REFS = f.read().replace(b"\r", b"").rstrip(b"\n")

class GitHTTPRequestHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        self.send_response(200)
        self.send_header("Content-Type", "application/x-git-upload-pack-advertisement")
        self.send_header("Content-Length", str(len(INFO)))
        self.end_headers()
        self.wfile.write(INFO)

    def do_POST(self):
        if "Content-Encoding" in self.headers:
            self.send_error(418)
            return

        req = self.rfile.read(int(self.headers["Content-Length"]))
        self.send_response(200)
        self.send_header("Content-Type", "application/x-git-upload-pack-result")
        self.send_header("Content-Length", str(len(REFS)))
        self.end_headers()
        pos = 0
        for delay, count in TIMING:
            if delay != 0:
                delay = min(delay, MAXDELAY)
            time.sleep(delay / 1000000)
            self.wfile.write(REFS[pos : pos + count])
            self.wfile.flush()
            pos += count
        self.wfile.write(REFS[pos:])

if __name__ == "__main__":
    print("Starting server on localhost:" + PORT)
    HTTPServer(("127.0.0.1", PORT), GitHTTPRequestHandler).serve_forever()
