Module.getPreloadedPackage = function getCachedDownload(
  packageName,
  packageSize
) {
  return dataPack;
};

var dataPack = null;
var dbRequest = indexedDB.open("DB", 1);
dbRequest.onerror = function(event) {
  console.log("Error loading database");
};
dbRequest.onsuccess = function() {
  var db = dbRequest.result;
  var transaction = db.transaction("Store", "readwrite");
  var objectStore = transaction.objectStore("Store");
  var existingPackRequest = objectStore.get("datapack");
  existingPackRequest.onsuccess = function() {
    dataPack = existingPackRequest.result;
  };
  existingPackRequest.onerror = function(ev) {
    console.log(ev);

    // Do the do but we don't listen for the end since that's silly

    Module.preRun.push(function() {
      objectStore.put();
    });

    // objectStore.put(dataPack, "datapack");
  };
};
