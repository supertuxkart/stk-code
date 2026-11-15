// Restore the browser filesystem before C++ opens the player's profile/config.
Module['preRun'] = Module['preRun'] || [];
Module['preRun'].push(function () {
    ENV['HOME'] = '/persist';
    ENV['XDG_CONFIG_HOME'] = '/persist/config';
    ENV['XDG_DATA_HOME'] = '/persist/data';
    FS.mkdirTree('/persist');
    FS.mount(IDBFS, {}, '/persist');
    addRunDependency('restore-saves');
    // Restoration and writes share a queue, so hiding/quitting the page cannot
    // start another filesystem sync while the initial read is pending.
    let saveQueue = new Promise(function (resolve) {
        FS.syncfs(true, function (error) {
            if (error) console.warn('Saved data could not be restored:', error);
            removeRunDependency('restore-saves');
            resolve();
        });
    });
    Module['syncSaves'] = function () {
        const operation = saveQueue.then(function () {
            return new Promise(function (resolve, reject) {
                FS.syncfs(false, function (error) {
                    if (error) reject(error);
                    else resolve();
                });
            });
        });
        // Return the real result for the final quit flush, but recover the
        // queue after errors so later saves can still be attempted.
        saveQueue = operation.catch(function (error) {
            console.warn('Saved data could not be stored:', error);
        });
        return operation;
    };
    const flush = function () { Module['syncSaves']().catch(function () {}); };
    const timer = setInterval(flush, 5000);
    const onVisibilityChange = function () { if (document.hidden) flush(); };
    document.addEventListener('visibilitychange', onVisibilityChange);
    Module['disposeBrowserHooks'] = function () {
        clearInterval(timer);
        document.removeEventListener('visibilitychange', onVisibilityChange);
    };
});
