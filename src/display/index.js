const WinUtils = require('../../build/Release/win-utils');
/**
 * @typedef {Object} MonitorInfo
 * @property {boolean} isActive - Indicates if the monitor is active.
 * @property {number} [outputTechnology] - The video output technology type (e.g., HDMI, DisplayPort).
 * @property {number} [scaling] - The display scaling mode.
 * @property {number} [rotation] - The display rotation (0, 90, 180, 270).
 * @property {boolean} [isStereo] - Indicates if the monitor supports stereo mode.
 * @property {boolean} [isInternal] - Indicates if the monitor is internal.
 * @property {number} id - Unique identifier for the monitor.
 * @property {number} positionX - The X coordinate of the monitor's position.
 * @property {number} positionY - The Y coordinate of the monitor's position.
 * @property {number} width - The width of the monitor in pixels.
 * @property {number} height - The height of the monitor in pixels.
 * @property {number} [workAreaX] - The X coordinate of the monitor's work area.
 * @property {number} [workAreaY] - The Y coordinate of the monitor's work area.
 * @property {number} [workAreaWidth] - The width of the monitor's work area.
 * @property {number} [workAreaHeight] - The height of the monitor's work area.
 * @property {boolean} [isPrimary] - Indicates if the monitor is the primary display.
 * @property {number} [edidManufactureId] - Manufacturer ID from EDID.
 * @property {number} [edidProductCodeId] - Product code ID from EDID.
 * @property {number} [connectorInstance] - Connector instance number.
 * @property {string} [monitorFriendlyDeviceName] - Friendly name of the monitor.
 * @property {string} [monitorDevicePath] - Device path of the monitor in the system.
 * @property {string} [szDevice] - Display device name (e.g., '\\\\.\\DISPLAY1').
 * @property {boolean} [isExtended] - Indicates if the monitor is part of an extended desktop.
 * @property {number} [refreshRate] - Refresh rate of the monitor.
 * @property {number} [sourceId] - Source ID.
 * @property {number} [targetId] - Target ID.
 * @property {number} [adapterIdHigh] - High part of the adapter ID.
 * @property {number} [adapterIdLow] - Low part of the adapter ID.
 * @property {number} [scalingPercentage] - The scaling percentage of the monitor (e.g., 100, 125, 150).
 */


module.exports = {
    /**
     * A function that returns information about the current monitors.
     * @returns {MonitorInfo[]} Array of monitor information objects.
     */
    getDisplaysInfo: () => WinUtils.getDisplayInfo(),
}