from http import server
import sys

port = 8000
if len(sys.argv) >= 2:
  port = int(sys.argv[1])

class MyHTTPRequestHandler(server.SimpleHTTPRequestHandler):
  def end_headers(self):
    self.send_header("Cross-Origin-Opener-Policy", "same-origin")
    self.send_header("Cross-Origin-Embedder-Policy", "require-corp")

    server.SimpleHTTPRequestHandler.end_headers(self)

if __name__ == '__main__':
  server.test(HandlerClass=MyHTTPRequestHandler, port=port)
