var a = 10;
var k = {};
k.subF1 = {}

k.subF1.subsubF1 = function() { 
    a;
    a++;/**bp:stack()**/
}
var m = k.subF1.subsubF1;
eval('m();');

function f0()
{
    var j0 =20;
    var j01 =20;
    
    function f1(a1)
    {
        var j = 10;
        var k = 20;
        var m = 20 + j0 + j01;
        function f2(b21, b22)
        {
            var j2 = arguments.length;
            var k2 = 10;
            function f3(c31,c32)
            {
                var a = 10;
                a++;                            /**bp:locals()**/
                a++;                            /**bp:evaluate('a');evaluate('j')**/
                return c31+c32+a;               /**bp:locals()**/
            }
            
            function f32()
            {
                j;
            }
            f3();
        }
        
        f2();
    }
    f1();
}

