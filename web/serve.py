#!/usr/bin/env python3
"""Serve a browser build on localhost with headers required by Wasm threads."""
import argparse
from functools import partial
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path


class Handler(SimpleHTTPRequestHandler):
    extensions_map = {**SimpleHTTPRequestHandler.extensions_map, ".wasm": "application/wasm"}

    def end_headers(self):
        # SharedArrayBuffer/pthreads require a cross-origin isolated page.
        self.send_header("Cross-Origin-Opener-Policy", "same-origin")
        self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
        self.send_header("Cache-Control", "no-cache")
        super().end_headers()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--directory", type=Path, default=Path(__file__).resolve().parents[1] / "build-web/web")
    parser.add_argument("--port", type=int, default=8080)
    args = parser.parse_args()
    if not (args.directory / "index.html").is_file():
        parser.error("Build output missing. Run web/build.py first.")
    # Bind only to this computer: this is a development server, not deployment.
    server = ThreadingHTTPServer(("127.0.0.1", args.port), partial(Handler, directory=str(args.directory.resolve())))
    print(f"Open http://localhost:{args.port}", flush=True)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        server.server_close()


if __name__ == "__main__":
    main()
