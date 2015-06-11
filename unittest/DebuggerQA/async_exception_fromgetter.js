
/**exception:stack();locals()**/

var a = {
	set value(val){
		return x_a++; /**bp:setExceptionResume('ignore')**/
		return 1;
	},
	
	get value(){
		return 1;
	}
}

var b = {
	get value(){
		WScript.SetTimeout(function(){
			x_b++; /**bp:setExceptionResume('ignore')**/
		},50);		
	},
}

b.value;

WScript.SetTimeout(function(){
	WScript.Echo(a.value++);
	WScript.Echo('Pass2');
},1);

WScript.SetTimeout(function(){
	b.value++; /**bp:setExceptionResume('ignore')**/
	WScript.Echo('Pass3')
},25);


