function test1() 
{
var a = {
'lowercase' : 'lowercase',
'lowerCase' : 'lowerCase',
'LowerCase' : 'LowerCase' };
var remoteTypeInfo = Debug.getTypeInfo(a);
};
function test2()
{
  var test = {
    lowercase: function() {return 0},
    lowerCase: function() { return 1},
    LowerCase: function() { return 2}
  };
  var typeInfo = Debug.getTypeInfo(test);
}

test1();
test2();
WScript.Echo("PASS");