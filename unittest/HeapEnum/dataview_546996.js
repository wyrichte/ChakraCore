var HETest = {};

HETest.obj = {};

HETest.obj['a dataview'] = new DataView(new ArrayBuffer(0));
HETest.obj[1] = new DataView(new ArrayBuffer(65536));
HETest.obj[5] = new DataView(new ArrayBuffer(1024), 512);
HETest.obj['another dataview'] = new DataView(new ArrayBuffer(2048), 48);

Debug.dumpHeap(HETest, /*dump log*/true, /*forbaselineCompare*/true, /*rootsOnly*/false, /*returnArray*/false);