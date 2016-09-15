/*
   Break on exception 
   Map throws TypeError
*/

/**exception:stack()**/

function Run() {
    var x = 1;
    {
        Map.prototype.has.call({});   /**bp:setExceptionResume('ignore')**/
    }
    x++;
    x; /**bp:locals()**/
    WScript.Echo('PASSED');
}

Run();
