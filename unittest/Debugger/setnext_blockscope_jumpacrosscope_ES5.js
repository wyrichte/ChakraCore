/*
   Jump across scopes 
*/

function Run() {
    for (var i = 0; i < 1; i++) {
        {
            var y = 2;;
            var x = 1; /**bp:setnext("bp1");locals(1)**/
            y = 200;
            {
                var y = "assigned";
                var x = 100;/**loc(bp1)**/
                x++;
            }
            x; /**bp:locals(1)**/
        }
    }
    WScript.Echo('PASSED');
}

WScript.Attach(Run);