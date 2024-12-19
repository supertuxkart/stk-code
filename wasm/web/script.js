import pako from "https://cdn.jsdelivr.net/npm/pako@2.1.0/+esm";
import jsUntar from "https://cdn.jsdelivr.net/npm/js-untar@2.0.0/+esm";

let db = null;
let db_name = "stk_db";
let store_name = "stk_store";
let idbfs_mount = null;
let data_version = 1;

let start_button = document.getElementById("start_button");
let status_text = document.getElementById("status_text");
let info_container = document.getElementById("info_container");
let quality_select = document.getElementById("quality_select");

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

function request_async(request) {
  return new Promise((resolve, reject) => {
    request.onerror = (event) => {
      reject(event);
    };
    request.onsuccess = () => {
      resolve(request.result);
    };
  });
}

async function delete_db() {
  if (db) {
    db.close();
    db = null;
  }
  await request_async(indexedDB.deleteDatabase(db_name));
}

async function load_store() {
  await load_db();
  let transaction = db.transaction(store_name, "readwrite");
  let store = transaction.objectStore(store_name);
  return store;
}

async function check_db(key) {
  let store = await load_store();
  return await request_async(store.count(key));
}

async function read_db(key) {
  let store = await load_store();
  return await request_async(store.get(key));
}

async function write_db(key, data) {
  let store = await load_store();
  return await request_async(store.put(data, key));
}

async function read_db_chunks(key) {
  let {size, chunk_count} = await read_db(key);
  let offset = 0;
  let array = new Uint8Array(size);
  
  for (let i = 0; i < chunk_count; i++) {
    let chunk_array = await read_db(key + "." + i);
    array.set(chunk_array, offset);
    offset += chunk_array.length;
  }

  return array;
}

async function write_db_chunks(key, data) {
  let size = data.length;
  let chunk_size = 20_000_000;
  let chunk_count = Math.ceil(size / chunk_size);

  let offset = 0;
  for (let i = 0; i < chunk_count; i++) {
    let chunk_array = data.slice(offset, offset + chunk_size);
    await write_db(key + "." + i, chunk_array);
    offset += chunk_size;
  }

  await write_db(key, {size, chunk_count});
}

async function download_chunks(url) {
  let path = url.split("/");
  let filename = path.pop();
  let base_url = path.join("/");

  status_text.textContent = `Downloading manifest for ${filename}...`;
  let r1 = await fetch(url + ".manifest");
  let manifest = (await r1.text()).split("\n");
  let size = parseInt(manifest.shift());
  manifest.pop();

  let offset = 0;
  let chunk = null;
  let array = new Uint8Array(size);
  let chunk_count = manifest.length;
  let current_chunk = 1;

  while (chunk = manifest.shift()) {
    let mb_progress = Math.floor(offset / (1024 ** 2))
    let mb_total = Math.floor(size / (1024 ** 2))
    status_text.textContent = `Downloading ${filename}... (chunk ${current_chunk}/${chunk_count}, ${mb_progress}/${mb_total}MiB)`;

    let r2 = await fetch(base_url + "/" + chunk);
    let buffer = await r2.arrayBuffer();
    let chunk_array = new Uint8Array(buffer);
    array.set(chunk_array, offset);
    offset += chunk_array.length;  
    current_chunk++;
  }

  return array.buffer;
}

async function extract_tar(url, fs_path, use_cache = false) {
  //download tar file from server and decompress
  let decompressed;
  if (!use_cache || !await check_db(url)) {
    let filename = url.split("/").pop();
    let compressed = await download_chunks(url);
    status_text.textContent = `Decompressing ${filename}...`;
    decompressed = pako.inflate(compressed);
    compressed = null;
    if (use_cache) {
      status_text.textContent = `Saving ${filename} to the cache...`;
      await write_db_chunks(url, decompressed);
    }
  }
  else {
    decompressed = await read_db_chunks(url);
  }

  //read the tar file and add to emscripten's fs
  let files = await jsUntar(decompressed.buffer);
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
  //check if we need to update the assets bundle
  if (!await check_db("/version") || !(await read_db("/version") == data_version)) {
    await delete_db();
    await write_db("/version", data_version);
  }

  let quality = quality_select.value;
  let data_url = `/game/data_${quality}.tar.gz`;
  await extract_tar(data_url, "/data", true);
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

function wait_for_frame() {
  return new Promise((resolve) => {requestAnimationFrame(resolve)});
}

async function main() {
  globalThis.ready = true;
  await load_idbfs();
  start_button.onclick = start_game;
  start_button.disabled = false;
  status_text.innerText = "";
}

async function start_game() {
  status_text.textContent = "Loading game files...";
  start_button.disabled = true;
  await load_data();
  await wait_for_frame();
  status_text.textContent = "Launching game...";

  await wait_for_frame();
  run();
  info_container.style.zIndex = 0;
  info_container.style.display = "none";
  sync_idbfs();

  console.warn("Warning: Opening devtools may harm the game's performance.");
}

Module["canvas"] = document.getElementById("canvas")
globalThis.main = main;
globalThis.sync_idbfs = sync_idbfs;
globalThis.load_idbfs = load_idbfs;

let poll_runtime_interval = setInterval(() => {
  if (globalThis.ready) {
    main();
    clearInterval(poll_runtime_interval);
  }
}, 100);