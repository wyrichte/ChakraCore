/**ref:WeakMapCopyOnWrite1.ref.js**/
map.a = 1; /* 1.) Copy the map */
key.a = 1; /* 2.) Copy the key - in the WeakMapKeyMap translation - we should be mapping to the right map */
map.get(key)./**ml:toExponential**/