// Test various assignment operators with constants on the LHS

var startGroup = '---------------------';

// x += y
WScript.Echo(startGroup);
try {
    "plus-equals" += 4;
}
catch(e) {
}
try {
    0 += 1;
}
catch(e) {
}
try {
    1.1 += 2;
}
catch (e) {
}
WScript.Echo("plus-equals");
WScript.Echo(0);
WScript.Echo(1.1);

// x -= y
WScript.Echo(startGroup);
try {
    "minus-equals" -= 4;
}
catch (e) {
}
try {
    0 -= 1;
}
catch (e) {
}
try {
    1.1 -= 2;
}
catch (e) {
}
WScript.Echo("minus-equals");
WScript.Echo(0);
WScript.Echo(1.1);

// x &= y
WScript.Echo(startGroup);
try {
    "and-equals" &= 4;
}
catch (e) {
}
try {
    0 &= 1;
}
catch (e) {
}
try {
    1.1 &= 2;
}
catch (e) {
}
WScript.Echo("and-equals");
WScript.Echo(0);
WScript.Echo(1.1);

// x++
WScript.Echo(startGroup);
try {
    "post-inc"++;
}
catch (e) {
}
try {
    0++;
}
catch (e) {
}
try {
    1.1++;
}
catch (e) {
}
WScript.Echo("post-inc");
WScript.Echo(0);
WScript.Echo(1.1);

// x--
WScript.Echo(startGroup);
try {
    "post-dec"--;
}
catch (e) {
}
try {
    0--;
}
catch (e) {
}
try {
    1.1--;
}
catch (e) {
}
WScript.Echo("post-dec");
WScript.Echo(0);
WScript.Echo(1.1);

// ++x
WScript.Echo(startGroup);
try {
    ++"pre-inc";
}
catch (e) {
}
try {
    ++0;
}
catch (e) {
}
try {
    ++1.1;
}
catch (e) {
}
WScript.Echo("pre-inc");
WScript.Echo(0);
WScript.Echo(1.1);

// --x
WScript.Echo(startGroup);
try {
    --"pre-dec";
}
catch (e) {
}
try {
    --0;
}
catch (e) {
}
try {
    --1.1;
}
catch (e) {
}
WScript.Echo("pre-dec");
WScript.Echo(0);
WScript.Echo(1.1);
