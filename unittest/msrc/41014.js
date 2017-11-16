var name1="e=function(){print('pass');};"+"  ".repeat(0x3f9);

(function()
{
  var o = {};
  o[name1]=0;
  for(var x in o)
  {
    eval(x);
  }
  delete o;
  o=0;
  CollectGarbage();
})();

for( var i=0; i<100;i++)
{
  CollectGarbage();
  WScript.LoadScript( 'eval(name1);');
}

e();