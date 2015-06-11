// Tests that function declaration bindings show correctly for the
// global function case where shadowing occurs from above
// (with no var binding created).
// Bug 305562.

function print(e){WScript.Echo(e)}
(function ()
{
    for (var i = 0; i < 2; i += 1)
    {
        try
        {
            (function ()
            {
                "use strict";

                if (i == 0)
                    f(); /**bp:evaluate('f')**/

                if (true)
                {
                    function f() { }
                    f(); /**bp:evaluate('f')**/
                }

                f(); /**bp:evaluate('f')**/
            })();
        }
        catch (e)
        {
            e; /**bp:evaluate('e')**/
        }
    }
})();

WScript.Echo("PASSED");