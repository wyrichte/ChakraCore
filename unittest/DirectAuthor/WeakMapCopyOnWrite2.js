/**ref:WeakMapCopyOnWrite2.ref.js**/
var key = module.getKey(); 
key.a = 1;                 /* 1.) Copy the key - in the WeakMapKeyMap translation - the map is not there - alright - will will build it during the translation */
var map = module.getMap(); /* 2.) Copy the map - now CopyOnWrite should already by completed */
map.get(key)./**ml:toExponential**/