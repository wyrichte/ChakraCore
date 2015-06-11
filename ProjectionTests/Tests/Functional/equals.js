WScript.Echo("start0");
WScript.InitializeProjection();
WScript.Echo("start");
var ps = new Windows.Foundation.Collections.PropertySet();
ps.addEventListener("mapchanged", function(ev){
WScript.Echo(ev);
WScript.Echo(ev.target);
WScript.Echo(ev.target !== undefined);
WScript.Echo(ev.target != undefined);
WScript.Echo(ev.target === undefined);
WScript.Echo(ev.target == undefined);
})
ps[0]=null;
