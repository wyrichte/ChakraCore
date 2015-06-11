function f(a)
{
	var x = 0;
	x += a; /**loc(A): stack();
                           locals();
                           disableBp('A'); 
                           enableBp('B');  **/
        x++;    /**loc(B): stack();
                           locals();
                           enableBp('C');
                           resume('step_over');
                **/
	x++;    /**loc(C): stack(); locals(); disableBp('C') **/
              
}
f(3);
f(7); /**bp: enableBp('A'); **/
f(9); 
WScript.Echo("pass");
