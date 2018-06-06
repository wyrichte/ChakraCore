var arr = ['banana', 'apple', 'peach'];
var num = {valueOf: function() {return 0}};
function fillArrayViaToString(n) {
    fillArrayViaToString.toString();
    return [];
}
Function.prototype.toString = function(x) {
    arr.push(fillArrayViaToString);
    return num;
};
fillArrayViaToString(1).join();
arr.length = 64;
try{arr.join(":");}catch(err){}
try{arr.sort(undefined); } catch (err) {}
Array.prototype.indexOf.apply(arr,[{}]);
WScript.Echo('pass');
