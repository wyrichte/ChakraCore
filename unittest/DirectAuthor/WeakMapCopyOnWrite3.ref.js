var map1 = new WeakMap();
var map2 = new WeakMap();
var key = { };
map1.set(key, 10086);     // Using the same key in two WeakMap will lead to more than one entry in the WeakMapKeyMap
map2.set(key, 12580);     // and translating those will be just fine too
