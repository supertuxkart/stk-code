// preRun executes before C++ main. Restore saves first so profile/config loading
// sees the previous session rather than creating a new profile on every visit.
Module['preRun'] = Module['preRun'] || [];
Module['preRun'].push(function () {
    ENV['HOME'] = '/persist';
    ENV['XDG_CONFIG_HOME'] = '/persist/config';
    ENV['XDG_DATA_HOME'] = '/persist/data';
    FS.mkdirTree('/persist');
    FS.mount(IDBFS, {}, '/persist');
    addRunDependency('restore-saves');
    FS.syncfs(true, function (error) {
        if (error) console.warn('Saved data could not be restored:', error);
        removeRunDependency('restore-saves');
    });

    // Serialize flushes: overlapping IndexedDB sync operations can race.
    let syncing = false;
    let dirty = false;
    Module['syncSaves'] = function () {
        if (syncing) { dirty = true; return; }
        syncing = true;
        FS.syncfs(false, function (error) {
            syncing = false;
            if (error) console.warn('Saved data could not be stored:', error);
            if (dirty) { dirty = false; Module['syncSaves'](); }
        });
    };
    // Includes profiles/highscores written by ordinary C++ file streams.
    setInterval(Module['syncSaves'], 5000);
    document.addEventListener('visibilitychange', function () {
        if (document.hidden) Module['syncSaves']();
    });
});
