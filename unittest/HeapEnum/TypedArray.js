WScript.LoadScriptFile("Utils.js")

var HETest = {};

HETest.uint16Array200 = new Uint16Array(200);
HETest.uint16Array1 = new Uint16Array(1);

HETest.arrayBuffer = new ArrayBuffer(256);
HETest.arrayBufferInt8Array256 = new Int8Array(HETest.arrayBuffer);
HETest.arrayBufferInt32Array64 = new Int32Array(HETest.arrayBuffer);
HETest.arrayBufferFloat64Array32 = new Float64Array(HETest.arrayBuffer);
HETest.arrayBufferUint16Array128 = new Uint16Array(HETest.arrayBuffer);

HETest.dataView = new DataView(HETest.arrayBuffer);
HETest.dataViewWithByteOffset = new DataView(HETest.arrayBuffer, 128);
HETest.dataViewWithByteOffsetAndByteLength = new DataView(HETest.arrayBuffer, 192, 32);

testExpectation("HETest.uint16Array200.length == 200");
testExpectation("HETest.uint16Array200.byteLength == 400");
testExpectation("HETest.uint16Array200.BYTES_PER_ELEMENT == 2");

testExpectation("HETest.uint16Array1.length == 1");
testExpectation("HETest.uint16Array1.byteLength == 2");
testExpectation("HETest.uint16Array1.BYTES_PER_ELEMENT == 2");

testExpectation("HETest.arrayBufferInt8Array256.length == 256");
testExpectation("HETest.arrayBufferInt8Array256.byteLength == 256");
testExpectation("HETest.arrayBufferInt8Array256.BYTES_PER_ELEMENT == 1");

testExpectation("HETest.arrayBufferInt32Array64.length == 64");
testExpectation("HETest.arrayBufferInt32Array64.byteLength == 256");
testExpectation("HETest.arrayBufferInt32Array64.BYTES_PER_ELEMENT == 4");

testExpectation("HETest.arrayBufferFloat64Array32.length == 32");
testExpectation("HETest.arrayBufferFloat64Array32.byteLength == 256");
testExpectation("HETest.arrayBufferFloat64Array32.BYTES_PER_ELEMENT == 8");

testExpectation("HETest.arrayBufferUint16Array128.length == 128");
testExpectation("HETest.arrayBufferUint16Array128.byteLength == 256");
testExpectation("HETest.arrayBufferUint16Array128.BYTES_PER_ELEMENT == 2");

displayResult();

Debug.dumpHeap(HETest, /*dump log*/true, /*forbaselineCompare*/true, /*rootsOnly*/false, /*returnArray*/false);
