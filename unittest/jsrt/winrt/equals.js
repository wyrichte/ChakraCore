var ps = new Windows.Foundation.Collections.PropertySet();
var result;
ps.addEventListener("mapchanged", function(ev){
result = ev;
return ev;
})
ps[0]=null;
result.toString();
