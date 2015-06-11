function TwoProperty()
{
    this.a = 1;
    this.b = 2;
}

var obj = new TwoProperty();
WScript.Echo('PASSED');/**bp:evaluate('obj.a');evaluate('obj.b');**/
obj = new TwoProperty();
WScript.Echo('PASSED');/**bp:evaluate('obj.a');evaluate('obj.b');**/
obj = new TwoProperty();
WScript.Echo('PASSED');/**bp:evaluate('obj.a');evaluate('obj.b');**/
obj.c = 10;
WScript.Echo('PASSED');/**bp:evaluate('obj.a');evaluate('obj.b');evaluate('obj.c');**/

