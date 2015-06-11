var module = (function(){
  
  var pair = (function hide()
  {
    /* This is to ensure map is not in the scope, so we will not CopyOnWrite the map first */
    var map = new WeakMap();
    var key = { };
    map.set(key, 10086);
    return { map : map, key : key };
  })();

  return { getMap: ((function(){ return pair.map; })), getKey: ((function(){ return pair.key; })) };
}());