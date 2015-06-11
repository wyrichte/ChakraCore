function a(){
    return Array.prototype.slice.call(arguments);
}
var test = a.bind(undefined, 1).bind(undefined, 2);
test(); /**bp:locals();**/
WScript.Echo("pass");
