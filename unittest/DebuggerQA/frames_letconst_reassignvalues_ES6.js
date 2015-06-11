/*
	SetNextStatement
	Let/const variables pass by ref
*/


function Run() {
    let a = {
		toString: function(){
			return "a";
		}
	}    
    bar(a);
}

function bar(a) {
    let b = {
		toString: function(){
			return "b";
		}
	}
    baz(a,b);
}

function baz(a,b) {
    let c = "baz";
    let c1 =  c + a + b; 
	c;
    /**bp:
        locals(1);
        stack();
        setFrame(1);
		evaluate('b.toString = function(){return \'b1\'}');
        locals(1);
        setFrame(2);
        evaluate('a.toString = function(){return \'a1\'}');
		locals(1);
        setFrame(0);
        evaluate('c + a + b')
        **/

     WScript.Echo('PASSED');
}

Run();
