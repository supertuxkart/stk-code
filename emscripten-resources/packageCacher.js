var chunkSize = 133169152;

function cacheRemotePackage(
  db,
  packageName,
  packageData,
  packageMeta,
  callback,
  errback
) {
  var transaction_packages = db.transaction([PACKAGE_STORE_NAME], IDB_RW);
  var packages = transaction_packages.objectStore(PACKAGE_STORE_NAME);
  var chunkSliceStart = 0;
  var chunkCount = Math.floor(packageData.byteLength / chunkSize) + 1;
  var finishedChunks = 0;
  for (var chunkId = 0; chunkId < chunkCount; chunkId++) {
    var putPackageRequest = packages.put(
      packageData.slice(chunkSliceStart, (chunkSliceStart += chunkSize)),
      "package/" + packageName + "/" + chunkId
    );
    putPackageRequest.onsuccess = function(event) {
      finishedChunks++;
      if (finishedChunks == chunkCount) {
        var transaction_metadata = db.transaction(
          [METADATA_STORE_NAME],
          IDB_RW
        );
        var metadata = transaction_metadata.objectStore(METADATA_STORE_NAME);
        var putMetadataRequest = metadata.put(
          {
            uuid: packageMeta.uuid,
            chunkCount
          },
          "metadata/" + packageName
        );
        putMetadataRequest.onsuccess = function(event) {
          callback(packageData);
        };
        putMetadataRequest.onerror = function(error) {
          errback(error);
        };
      }
    };
    putPackageRequest.onerror = function(error) {
      errback(error);
    };
  }
}

/* Check if there's a cached package, and if so whether it's the latest available */
function checkCachedPackage(db, packageName, callback, errback) {
  var transaction = db.transaction([METADATA_STORE_NAME], IDB_RO);
  var metadata = transaction.objectStore(METADATA_STORE_NAME);
  var getRequest = metadata.get("metadata/" + packageName);
  getRequest.onsuccess = function(event) {
    var result = event.target.result;
    if (!result) {
      return callback(false, null);
    } else {
      return callback(PACKAGE_UUID === result.uuid, result);
    }
  };
  getRequest.onerror = function(error) {
    errback(error);
  };
}

function fetchCachedPackage(db, packageName, metadata, callback, errback) {
  var transaction = db.transaction([PACKAGE_STORE_NAME], IDB_RO);
  var packages = transaction.objectStore(PACKAGE_STORE_NAME);

  var chunksDone = 0;
  var totalSize = 0;
  var chunks = new Array(metadata.chunkCount);

  for (var chunkId = 0; chunkId < metadata.chunkCount; chunkId++) {
    var getRequest = packages.get("package/" + packageName + "/" + chunkId);
    getRequest.onsuccess = function(event) {
      if (metadata.chunkCount == 1) {
        callback(event.target.result);
      } else {
        chunksDone++;
        totalSize += event.target.result.byteLength;

        chunks.push(event.target.result);
        if (chunksDone == metadata.chunkCount) {
          if (chunksDone == 1) {
            callback(event.target.result);
          } else {
            var tempTyped = new Uint8Array(totalSize);
            var byteOffset = 0;
            for (var chunkId in chunks) {
              var buffer = chunks[chunkId];
              tempTyped.set(new Uint8Array(buffer), byteOffset);
              byteOffset += buffer.byteLength;
              buffer = undefined;
            }
            chunks = undefined;
            callback(tempTyped.buffer);
            tempTyped = undefined;
          }
        }
      }
    };
    getRequest.onerror = function(error) {
      errback(error);
    };
  }
}
