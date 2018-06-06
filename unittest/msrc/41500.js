var buf="\x00\x61\x73\x6d\x01\x00\x00\x00";
var name="";
for(var i=0;i<0x1;i++){
	name+="a";
}
name="\x00\x02\x01"+name;
for(var i=0;i<0x1000000;i++){
	buf+=name;
}

function createView(bytes) {
  const buffer = new ArrayBuffer(bytes.length);
  const view = new Uint8Array(buffer);
  for (let i = 0; i < bytes.length; ++i) {
    view[i] = bytes.charCodeAt(i);
  }
  return view;
}
var mod=new WebAssembly.Module(createView(buf));
var obj={};
obj.toString=()=>{var s1;
s1="";
for(var i=0;i<0x1000000;i++){
	s1+="a";
}
return s1.substring(1,0x2);
}
WebAssembly.Module.customSections(mod,obj);

WScript.Echo("PASS");
