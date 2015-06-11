
function fooCaller() {
    var x = this;
    return function () {
        {
            var x = new Date();
            {
                var x = {}
                {
                    var x = [];
                    {
                        var x = 1;
                        {
                            function x() {
                                return 'x';
                            }
                            x; /**bp:evaluate('x()')**/
                        }
                        x;/**bp:evaluate('x()')**/
                    }
                }
            }
        }
        x; /**bp:locals(2);stack()**/
    }   
}

fooCaller()();
WScript.Echo('PASSED');