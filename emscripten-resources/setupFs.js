Module.preRun.push(function() {
  console.log("Prerun:putting user data into memory");
  window.userDataMount = FS.mount(IDBFS, {}, "/home/web_user").mount;
  userDataMount.type.syncfs(window.userDataMount, true, function(err) {
    if (err) {
      throw err;
    }
    console.log("Read user data into memory");
  });
  window.userDataSync = function userDataSync() {
    function doSync() {
      window.isSyncing = true;
      window.userDataMount.type.syncfs(window.userDataMount, function(err) {
        if (err) {
          throw err;
        }
        if (window.needsSync) doSync();
        else window.isSyncing = false;
      });
    }

    if (!window.isSyncing) {
      doSync();
    } else {
      window.needsSync = true;
    }
  };
});
