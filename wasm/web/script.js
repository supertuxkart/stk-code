import pako from "https://cdn.jsdelivr.net/npm/pako@2.1.0/+esm";
import jsUntar from "https://cdn.jsdelivr.net/npm/js-untar@2.0.0/+esm";

let db = null;
let db_name = "stk_db";
let store_name = "stk_store";
let idbfs_mount = null;

let start_button = document.getElementById("start_button");
let status_text = document.getElementById("status_text");

function load_db() {
  if (db) return db;
  return new Promise((resolve, reject) => {
    let request = indexedDB.open(db_name, 1);
    request.onerror = (event) => {
      reject(event);
    };
    request.onsuccess = (event) => {
      db = event.target.result;
      resolve(db);
    };
    request.onupgradeneeded = (event) => {
      let db = event.target.result;
      db.createObjectStore(store_name);
    };
  });
}

async function load_store() {
  await load_db();
  let transaction = db.transaction(store_name, "readwrite");
  let store = transaction.objectStore(store_name);
  return store;
}

function check_db(key) {
  return new Promise(async (resolve, reject) => {
    let store = await load_store();
    let query = store.count(key);
    query.onsuccess = () => {
      resolve(query.result);
    }
    query.onerror = (event) => {
      reject(event);
    }
  });
}

function read_db(key) {
  return new Promise(async (resolve, reject) => {
    let store = await load_store();
    let query = store.get(key);
    query.onsuccess = () => {
      resolve(query.result);
    }
    query.onerror = (event) => {
      reject(event);
    }
  });
}

function write_db(key, data) {
  return new Promise(async (resolve, reject) => {
    let store = await load_store();
    let query = store.put(data, key);
    query.onsuccess = () => {
      resolve();
    }
    query.onerror = (event) => {
      reject(event);
    }
  });
}

async function extract_tar(url, fs_path, use_cache = false) {
  //download tar file from server and decompress
  let decompressed;
  if (!use_cache || !await check_db(url)) {
    let r = await fetch(url);
    let compressed = await r.arrayBuffer();
    decompressed = pako.inflate(compressed).buffer;
    compressed = null;
    if (use_cache) {
      console.log("saving to cache");
      await write_db(url, decompressed);
    }
  }
  else {
    decompressed = await read_db(url);
  }

  //read the tar file and add to emscripten's fs
  let files = await jsUntar(decompressed);
  for (let file of files) {
    let relative_path = file.name.substring(1);
    let out_path = fs_path + relative_path;
    if (out_path.endsWith("/")) {
      try {
        FS.mkdir(out_path);
      }
      catch {}
    }
    else {
      let array = new Uint8Array(file.buffer);
      FS.writeFile(out_path, array);
      file.buffer = null;
    }
  }
}

async function load_data() {
  console.log("downloading and extracting game data");
  await extract_tar("/game/data.tar.gz", "/data", true);
  console.log("downloading and extracting assets");
  await extract_tar("/game/assets.tar.gz", "/data", true);
  console.log("done")
}

async function load_idbfs() {
  idbfs_mount = FS.mount(IDBFS, {}, "/home/web_user").mount;
  await sync_idbfs(true);
}

function sync_idbfs(populate = false) {
  return new Promise((resolve, reject) => {
    idbfs_mount.type.syncfs(idbfs_mount, populate, (err) => {
      if (err) reject(err);
      else resolve();
    });
  })
}

async function main() {
  await load_data();
  await load_idbfs();
  start_button.onclick = () => {requestAnimationFrame(start_game)};
  start_button.disabled = false;
  status_text.textContent = "Ready";
}

function start_game() {
  start_button.disabled = true;
  status_text.textContent = "Initializing";
  requestAnimationFrame(() => {
    run();
    status_text.textContent = "Running";
    sync_idbfs();
  });
}

globalThis.pako = pako;
globalThis.jsUntar = jsUntar;
globalThis.load_data = load_data;
globalThis.sync_idbfs = sync_idbfs;

Module["canvas"] = document.getElementById("canvas")
main();