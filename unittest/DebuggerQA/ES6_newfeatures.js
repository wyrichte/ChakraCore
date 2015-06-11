var placeHolder = 1;
var arrow_a = () => {
	var arrow_b = () => {
        let arrow_c = 1;
        arrow_a; /**bp:stack();locals(1, LOCALS_FULLNAME);evaluate('arrow_c', 1, LOCALS_FULLNAME);evaluate('arrow_b', 1, LOCALS_FULLNAME);evaluate('arrow_a', 1, LOCALS_FULLNAME)**/        
	}	
    arrow_b;
    arrow_b(); /**bp:evaluate('arrow_b', 2, LOCALS_FULLNAME)**/
    arrow_b;
}
arrow_a();

class classOne {
    constructor () {
        placeHolder;
        placeHolder; /**bp:stack();locals(0, LOCALS_FULLNAME);evaluate('constructor', 1, LOCALS_FULLNAME)**/
    }
    
    sample2 () {
       placeHolder;
       placeHolder; /**bp:stack();evaluate('sample2', LOCALS_FULLNAME)**/
    }    
}

class classTwo extends classOne {        
    constructor() {
        
        placeHolder;
        placeHolder; /**bp:locals(0, LOCALS_FULLNAME);evaluate('constructor', 1, LOCALS_FULLNAME)**/
    }
    
    sample1 () {
        placeHolder;
        placeHolder; /**bp:stack();evaluate('sample1', LOCALS_FULLNAME)**/
    }    
}

var _class2 = new classTwo();
_class2.sample1();
_class2.sample2();
WScript.Echo('pass');