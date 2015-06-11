

function write(a) {
    if (this.WScript == undefined) {
        document.write(a);
        document.write("</br>");
    }
    else
        WScript.Echo(a)
}
//regular function decl and call
function x(){};
function x(){ write("in function x");}
x();

var foo;


try
{
   eval("function foo::bar() {}");
} 
catch(ex) 
{
   write(ex);
}

// this should compile and work in v5.8 mode
// not implemented in Eze yet
// function a,b,c(){ write("in function m");}
// a();
// b();
// c();

try
{
	eval("try {} catch(y) { function y::window() {}}");
}
catch(ex)
{
   write(ex);
}