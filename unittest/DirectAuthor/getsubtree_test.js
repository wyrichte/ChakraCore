class Polygon {
    foo1() { 
    }
    
    static foo2() {
       /**subtree:-1**/
    }
    /**subtree:1**/
    foo3() {
    }
    constructor(a) {
        
    /**subtree:-1**/
        if (a > 1)
            this.x = a;
           
    }
}

var strExpr4 = `hello ${
/**subtree:-1**/function () { return "world" } 
()} !`