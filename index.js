const { getActiveAudioOutputSync, getActiveAudioOutput } = require('./src/audio');
const { getRemovableDrives, setDiskAttribute } = require('./src/usb');

module.exports = {
    //audio
    getActiveAudioOutputSync,
    getActiveAudioOutput,

    //usb
    getRemovableDrives,
    setDiskAttribute
}