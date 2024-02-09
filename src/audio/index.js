const WinUtils = require('../../build/Release/win-utils');

// Example
// audio.getActiveAudioOutputCallback((r) => {
//     console.log(r)
// })
// const driverName = audio.getActiveAudioOutputSync()

module.exports = {
    /**
     * A function that waits until a condition becomes true and then calls a callback (the callback and condition can be either sync or async)
     * @param {(arg: string) => void} callback - callback called with name of current audio device or with "" if detection failed.
     * @returns {void}
     */
    getActiveAudioOutput: async (callback) => WinUtils.getActiveAudioOutputAsync(callback),
    /**
     * A function that waits until a condition becomes true and then calls a callback (the callback and condition can be either sync or async)
     * @param - callback called with name of current audio device or with "" if detection failed.
     * @returns {string}
     */
    getActiveAudioOutputSync: () => WinUtils.getActiveAudioOutputSync(),
}