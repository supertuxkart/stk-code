from http import server

class MyHTTPRequestHandler(server.SimpleHTTPRequestHandler):
  def end_headers(self):
    self.send_header("Cross-Origin-Opener-Policy", "same-origin")
    self.send_header("Cross-Origin-Embedder-Policy", "require-corp")

    server.SimpleHTTPRequestHandler.end_headers(self)

if __name__ == '__main__':
  server.test(HandlerClass=MyHTTPRequestHandler)
