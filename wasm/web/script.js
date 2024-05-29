import pako from "https://cdn.jsdelivr.net/npm/pako@2.1.0/+esm";
import jsUntar from "https://cdn.jsdelivr.net/npm/js-untar@2.0.0/+esm";

async function extract_tar(url, fs_path) {
  //download tar file from server and decompress
  let r = await fetch(url);
  let compressed = await r.arrayBuffer();
  let decompressed = pako.inflate(compressed);
  let files = await jsUntar(decompressed.buffer);
  
  //save each file to the emscripten filesystem
  for (let file of files) {
    let relative_path = file.name.substring(1);
    let out_path = fs_path + relative_path;
    if (out_path.endsWith("/")) {
      FS.mkdir(out_path);
    }
    else {
      let array = new Uint8Array(file.buffer);
      FS.writeFile(out_path, array);
    }
  }
}

async function load_data() {
  await extract_tar("/game/data.tar.gz", "/data");
}

globalThis.pako = pako;
globalThis.jsUntar = jsUntar;
globalThis.load_data = load_data;