/*
    jump across blockscopes containing a let - negative scenario
*/



function Run(){
    {
        let y = 2;;
        let x = 1; /**bp:setnext("bp1");locals(1)**/
        y = 200;
        {
            let y = "assigned";
            let x = 100;/**loc(bp1)**/
            x++;
        }
        x; /**bp:locals(1)**/
    }
    WScript.Echo('PASSED');
}
WScript.Attach(Run);