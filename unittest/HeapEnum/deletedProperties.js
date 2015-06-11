
var HETest = {};

HETest.HE1 = {};
HETest.HE1.testDeleteFirstOfThree_1 = {};
HETest.HE1.testDeleteFirstOfThree_2 = {};
HETest.HE1.testDeleteFirstOfThree_3 = {};
delete HETest.HE1.testDeleteFirstOfThree_1;

HETest.HE2 = {};
HETest.HE2.testDeleteSecondThree_1 = {};
HETest.HE2.testDeleteSecondOfThree_2 = {};
HETest.HE2.testDeleteSecondOfThree_3 = {};
delete HETest.HE2.testDeleteSecondOfThree_2;

HETest.HE3 = {};
HETest.HE3.testDeleteThirdThree_1 = {};
HETest.HE3.testDeleteThirdOfThree_2 = {};
HETest.HE3.testDeleteThirdOfThree_3 = {};
delete HETest.HE3.testDeleteThirdOfThree_3;

HETest.HE4 = {};
HETest.HE4.testDeleteAllOfThree_1 = {};
HETest.HE4.testDeleteAllOfThree_2 = {};
HETest.HE4.testDeleteAllOfThree_3 = {};
delete HETest.HE4.testDeleteAllOfThree_1;
delete HETest.HE4.testDeleteAllOfThree_2;
delete HETest.HE4.testDeleteAllOfThree_3;

HETest.HE5 = {};
HETest.HE5.testDeleteNested = {};
HETest.HE5.testDeleteNested.NestLevel2 = {};
HETest.HE5.testDeleteNested.NestLevel2.NestLevel3 = {};
delete HETest.HE5.testDeleteNested.NestLevel2;

Debug.dumpHeap(HETest, true);


