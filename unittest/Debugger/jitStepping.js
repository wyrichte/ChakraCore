var print;
if(typeof(WScript) == "undefined")
{
	if(console.log)
	{
	print = console.log;
	}
}
else if (typeof(print) == "undefined")
{
    print = WScript.Echo;
}

function inner()
{
	var infoo = true;
	print("In inner. Jit:", Debug.isInJit());
}

function innerMost()
{
    print("In innermost. Jit:", Debug.isInJit());
}

function outer()
{
	print("In outer"); 
	print("In jit:", Debug.isInJit());
	inner();
	inner(); /**bp:stack();resume('step_into'); stack();**/
	inner(); /**bp:stack();resume('step_over'); stack();**/
	inner(); 
	innerMost();
	innerMost(); /**bp:stack();resume('step_into'); resume('step_into'); stack();**/
	inner(); /**bp:stack();resume('step_out'); resume('step_over');stack(); **/
}

print("In global function: Before outer...Jit:", Debug.isInJit());
outer();
print("Global again");
print("In jit:", Debug.isInJit());
