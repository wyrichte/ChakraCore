// x86_debug
//Switches:-maxinterpretcount:1 -force:fieldcopyprop -off:simplejit
function test5() {
  for (var e in Array([aa] = '', String('dd'), Object())) {
  }
}
Object.preventExtensions(this);
test5();
test5();

(function() {
  var foo = function () { };
  var bar = function() {}
  var zee = function() {}
  Object.preventExtensions(this); 

  function test4() {
      foo(zee(), bar([qbtdan] = "u4E8B")); 
      foo(bar([qbtdan] = "u4E8B")); 
      bar([qbtdan] = "u4E8B", zee()); 
      foo(bar([qbtdan] = "u4E8B", zee())); 
    };


  //Profile

  test4();
  test4();
  test4();
  test4();
  test4();
  test4();
  test4();
})();

print('pass');
