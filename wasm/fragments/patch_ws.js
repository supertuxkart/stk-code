/* REPLACE
ws ?= ?new WebSocketConstructor\(url, ?opts\)
*/
if (globalThis.config.ws_enabled) {
  ws = new WebSocket(url);
}
else {
  console.error("websocket creation denied - disabled by config");
  throw new TypeError("ws disabled");
}
