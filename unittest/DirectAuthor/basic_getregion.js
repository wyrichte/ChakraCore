(function()
{
    try
    {
        for (var i = 0; i < 10; i++)
        {
            do
            {
            } 
            while (false);
            while (false)
            {
            }
            for (var j in i)
            {
                var x = { "a" : i, "b" : j };
                var y = [ "a", "b", "c" ];
            }
        }
    }
    catch (e)
    {
    }
    finally
    {
    }
})();

/**gr:**/