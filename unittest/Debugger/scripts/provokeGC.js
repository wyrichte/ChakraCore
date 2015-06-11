this.holder = [];
for (var j = 0; j < 10; j++) {
  eval("var i" + j + " = " + j);
}
CollectGarbage();
this.holder = null;