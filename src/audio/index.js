const WinUtils = require('../../build/Release/win-utils');

// Example
// audio.getActiveAudioOutputCallback((r) => {
//     console.log(r)
// })
// const driverName = audio.getActiveAudioOutputSync()

module.exports = {
    /**
     * A function that return current audio output device name
     * @param {(arg: string) => void} callback - callback called with name of current audio device or with "" if detection failed.
     * @returns {void}
     */
    getActiveAudioOutput: async (callback) => WinUtils.getActiveAudioOutputAsync(callback),
    /**
     * A function that return current audio output device name
     * @returns {string}
     */
    getActiveAudioOutputSync: () => WinUtils.getActiveAudioOutputSync(),
}