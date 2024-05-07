const WinUtils = require('../../build/Release/win-utils');

// const getGetDisks = async () => {
//     console.log(await getRemovableDrives());
//     console.log(await setDiskAttribute("F", 0));
// }

module.exports = {
    /**
     * A function that return all removable drives
     * @returns {string[]} Array of removable devices
     */
    getRemovableDrives: async () => WinUtils.getRemovableDrives(),
    /**
     * A function that return current audio output device name
     * @param {string} diskLetter - removable disk letter.
     * @param {number} attributes - attributes for disk (0 - set writable, 1 - set read-only).
     * @returns {boolean} true = success, false = failure
     */
    setDiskAttribute: async (diskLetter, attributes) => WinUtils.setDiskAttribute(diskLetter, attributes),
}