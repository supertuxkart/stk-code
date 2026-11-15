"""Verify actual HTTP delivery of threaded WebAssembly builds."""
from functools import partial
from http.server import ThreadingHTTPServer
from pathlib import Path
import tempfile
import threading
import unittest
from urllib.request import urlopen

from serve import Handler


class BrowserServerTest(unittest.TestCase):
    def test_wasm_is_served_with_thread_isolation_headers(self):
        # A successful HTML response alone does not prove that the browser will
        # accept a shared-memory Wasm module. Check the module response as well.
        with tempfile.TemporaryDirectory() as directory:
            wasm = b"\x00asm\x01\x00\x00\x00"
            Path(directory, "game.wasm").write_bytes(wasm)
            server = ThreadingHTTPServer(("127.0.0.1", 0), partial(Handler, directory=directory))
            worker = threading.Thread(target=server.serve_forever, daemon=True)
            worker.start()
            try:
                with urlopen(f"http://127.0.0.1:{server.server_port}/game.wasm", timeout=5) as response:
                    self.assertEqual(response.read(), wasm)
                    self.assertEqual(response.headers.get_content_type(), "application/wasm")
                    self.assertEqual(response.headers["Cross-Origin-Opener-Policy"], "same-origin")
                    self.assertEqual(response.headers["Cross-Origin-Embedder-Policy"], "require-corp")
            finally:
                server.shutdown()
                server.server_close()
                worker.join(timeout=5)
