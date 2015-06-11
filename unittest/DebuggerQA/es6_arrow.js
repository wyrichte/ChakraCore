
var arrow_expression = () => this;
var arrow_concisebody = () => { this; }


arrow_expression()/**bp:resume('step_into');locals();stack();resume('step_over');locals()**/
this;
arrow_concisebody()/**bp:resume('step_into');locals();stack();resume('step_over');locals()**/

var deepfunction = function() {

    (() => {
    
        (function() {
            
            (() => {
                
                var a = 1; /**bp:stack()**/
                a;
                a;
                a; /**bp:locals();evaluate('this',1)**/
                a;
                
            })();
        
        })()
    
    })()
}

deepfunction();


var deepfunctionwithNames = function() {

    var l2 = () => {
    
        var l3 = function() {
            
            var l4 = () => {
                
                var a = 1; 
                a;
                a; /**bp:stack()**/
                a;
                
            };
            l4();
        
        };
        l3();
    
    };
    
    l2();
}

deepfunctionwithNames();


var obj = {
    fn : function(){
        this.lambda2();
    },
    lambda2 : () => {
        this; /**bp:stack()**/
    }
}

obj.fn();


var noeval = () => {
        var a = 1;
        a;
        a; /**bp:evaluate('this',1)**/
        a;
}
noeval();

WScript.Echo('pass');