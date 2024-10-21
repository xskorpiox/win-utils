const { getActiveAudioOutputSync, getActiveAudioOutput } = require('./src/audio');
const { getRemovableDrives, setDiskAttribute } = require('./src/usb');
const { getWindowHWNDbyTitle, emitMessageToWindow, closeWindowByTitle } = require('./src/system');
const { getDisplaysInfo } = require('./src/display');

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
    closeWindowByTitle,

    //display
    getDisplaysInfo,
}