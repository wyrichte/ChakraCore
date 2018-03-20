var pageSize = 64 * 1024;
function makeSharedArrayBuffer(length) {
    const pages = Math.ceil(length / pageSize);
    const mem = new WebAssembly.Memory({initial: pages, maximum: pages, shared: true});
    if (!(mem.buffer instanceof SharedArrayBuffer)) {
        throw new Error("WebAssembly.Memory::buffer is not a SharedArrayBuffer");
    }
    return mem.buffer;
}

this.WScript.LoadScriptFile("./sharableBuffer.js");
