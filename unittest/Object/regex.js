
function hello()
{
	var regex = /blah/;
	WScript.Echo("blah: " + regex.blah);
	regex.blah = 1;
	WScript.Echo("blah: " + regex.blah);
}

hello();
hello();
