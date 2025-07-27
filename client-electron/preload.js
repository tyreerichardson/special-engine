const { contextBridge, ipcRenderer } = require('electron')

/*
* script that exposes selected properties of Electron's process.
* versions object to the renderer process in a versions global variable.
*/
contextBridge.exposeInMainWorld('versions', {
    node: () => process.versions.node,
    chrome: () => process.versions.chrome,
    electron: () => process.versions.electron,
    ping: () => ipcRenderer.invoke('ping')
    // we can also expose variables, not just functions
})