const { getActiveAudioOutputSync, getActiveAudioOutput } = require('./src/audio');
const { getRemovableDrives, setDiskAttribute } = require('./src/usb');
const { getWindowHWNDbyTitle, emitMessageToWindow, closeWindowByTitle } = require('./src/system');

module.exports = {
    //audio
    getActiveAudioOutputSync,
    getActiveAudioOutput,

    //usb
    getRemovableDrives,
    setDiskAttribute,

    //system
    getWindowHWNDbyTitle,
    emitMessageToWindow,
    closeWindowByTitle
}