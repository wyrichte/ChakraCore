var HETest = {};
HETest.s = "GGCCGGGTAAAGTGGCTCACGCCTGTAATCCCAGCACTTTACCCCCCGAGGCGGGCGGA";
HETest.result = HETest.s.match(/[cgt]gggtaaa|tttaccc[acg]/ig);

Debug.dumpHeap(HETest, /*dump log*/true, /*forbaselineCompare*/true, /*rootsOnly*/false, /*returnArray*/false);
