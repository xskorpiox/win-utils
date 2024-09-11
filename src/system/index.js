const WinUtils = require('../../build/Release/win-utils');

const WM_CLOSE = 0x0010; // message for closing window

module.exports = {
    /**
     * A function that return current window HWND
     * @param {string} title - window title.
     * @returns {number} return HWND, 0 === not exists
     */
    getWindowHWNDbyTitle: async (title) => WinUtils.getWindowHWNDbyTitle(title),
    /**
     * Sends a message to a window by its HWND.
     *
     * @function sendMessage
     * @param {Number} hwnd - The window identifier (HWND) to which the message should be sent.
     * @param {Number} msg - The message code to be sent (e.g., WM_CLOSE, WM_KEYDOWN, etc.).
     * @param {Number} wParam - Additional message parameter (depends on the message type).
     * @param {Number} lParam - Additional message parameter (depends on the message type).
     * @returns {Number} - The result of the SendMessage function, usually the return code from the window's message handler.
     * @throws {TypeError} - If one of the arguments has an incorrect type.
     */
    emitMessageToWindow: async (hwnd, msg, wParam = 0, lParam = 0) => WinUtils.sendMessageToWindow(hwnd, msg, wParam, lParam),
    /**
     * A function that close window with specified title
     * @param {string} title - window title.
     * @returns {number} 0 === success, -1 === window not found;
     */
    closeWindowByTitle: (title) => {
        const hwnd = WinUtils.getWindowHWNDbyTitle(title);

        if (!hwnd) {
            return -1;
        }

        return WinUtils.sendMessageToWindow(hwnd, WM_CLOSE, 0, 0)
    },
}