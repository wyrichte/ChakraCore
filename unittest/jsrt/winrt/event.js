// NOTE: The map changed event fires sync to the call.
// We expect the result and the 'async' injectedResult to be 'map changed'
injectedResult = 'result is pending';
var ps = new Windows.Foundation.Collections.PropertySet();
ps.addEventListener("mapchanged", function(ev){
  injectedResult = 'map changed';
});
ps[0]=null;
injectedResult.toString();