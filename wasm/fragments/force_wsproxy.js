/* INSERT
var ?opts ?= ?undefined;
*/
var parts = addr.split("/");
if (!url.endsWith("/")) url += "/";
url += parts[0] + ":" + port;

/* REPLACE
url ?= ?SOCKFS\.websocketArgs\[['"]url['"]\];
*/
var parts = addr.split("/");
url = Module.websocket.url;
if (!url.endsWith("/")) url += "/";
url += parts[0] + ":" + port;

