// uses es6 string template as validation for ES6 'stable' features

function echo(str) {
	WScript.Echo(str);
}

var world="WoRlD!";
echo(`hello${world}`);

