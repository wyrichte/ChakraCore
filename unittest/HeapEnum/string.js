var HETest = {};

HETest.stringObjectProperty = new String("StringObjectValue");
HETest.stringObjectPropertyFromStringObject = new String(HETest.stringObjectProperty);
HETest.stringLiteralPropertyShort = "stringLiteralValueShort";
HETest.stringLiteralPropertyLong = "stringLiteralValueLong has a longer string literal value than stringLiteralValueShort";
HETest.stringObjectPropertyFromStringLiteral = new String(HETest.stringLiteralPropertyShort);
HETest.emptyStringObjectProperty = new String();

var n = 5;
var x = new Array(n);

var count = 1;
for(var i = 0; i < n; ++i)
{
	var c = String.fromCharCode(97+i);
	x[i] = "";

	for(var j = 0; j < count; ++j)
	{
		x[i] += c;
	}
	
	count *= 3;
}

HETest.concatStr = x[0]; // HETest.concatStr[0]
HETest.concatStr += x[1] + x[2]; // HETest.concatStr[1-12]
HETest.concatStr += "XXXX"; // HETest.concatStr[13-16]
HETest.concatStr += x[3] + "XXXX"; // HETest.concatStr[17-47]
HETest.concatStr += HETest.concatStr + x[4] + HETest.concatStr + x[4]; // HETest.concatStr[48-305]
HETest.concatStr += HETest.concatStr + x[0] + HETest.concatStr; // HETest.concatStr[306-822]
HETest.concatStr += "XXXX"; // HETest.concatStr[823-826]

HETest.copyOfConcatStr = new String(HETest.concatStr);

HETest.concatStrToFlatten = "a";
HETest.concatStrToFlatten += "bbb" + "ccccccccc";
HETest.concatStrToFlatten += "XXXX";
HETest.concatStrToFlatten += "ddddddddddddddddddddddddddd" + "XXXX";

HETest.sliceConcatStr = HETest.concatStrToFlatten.slice(3, 45); // "bcccccccccXXXXdddddddddddddddddddddddddddX"
HETest.substringConcatStr = HETest.concatStrToFlatten.substring(10, 30); // "cccXXXXddddddddddddd"

HETest.concatStrFromStrings = HETest.concatStrToFlatten + HETest.sliceConcatStr + HETest.substringConcatStr;

HETest.sliceStringLiteralEmpty = "testing string".slice(-1, 2);
HETest.sliceStringLiteralSingle = "another testing string".slice(-1);
HETest.sliceStringLiteralMultiple = "yet another testing string".slice(-10);

HETest.singleCharStringLetter = new String('a');
HETest.singleCharStringDigit = new String(0);
HETest.singleCharStringLiteral = 'b';

Debug.dumpHeap(HETest, /*dump log*/true, /*forbaselineCompare*/true, /*rootsOnly*/false, /*returnArray*/false);
