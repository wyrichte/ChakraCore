// from the first iframe1

var a = 10;
a++;
eval('a--;');       /**bp:stack();evaluate('a');**/

function test2()
{
    var m = 10; /**bp:stack();setFrame(2);locals()**/
}
