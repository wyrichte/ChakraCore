var testObjects = [
    {
        get: function(){
            return new ImageData({
                width: 12,
                height: 34,
                data: [1, 2, 3, 4],
                type: "text/jpeg",
                size: 4096,
                compression: 0.7,
                lastModifiedDate: new Date("2011-11-15T13:42:42.658Z")
            });
        }
    },

    {
        get: function(){
            var img = new ImageData({
                width: 123,
                height: 3456,
                data: [2, 3, 4, 5, 6, 7, 8, 9], // PixelArray verifies length % 4 == 0
                type: "text/png",
                size: 128,
                compression: 0.4,
                lastModifiedDate: new Date("2011-11-15T13:44:00.000Z")
            });
            var root = {a:{a1:null},b:{b1:undefined}};
            root.a.a2 = img;
            root.b.b2 = img;
            root.b.b3 = img;
            return root;
        }
    },
];

WScript.LoadScriptFile("sca_lib.js");

(function()
{
    for (p in testObjects)
    {
        var test = testObjects[p];
        var obj = getTestObject(test);
    
        var dump = fmt.stringify(obj);
        echo(dump);
    
        guarded_call(function(){
            var blob = SCA.serialize(obj, {context: "crossprocess"});
            fmt.print(blob);
    
            var obj2 = SCA.deserialize(blob);
            var blob2 = SCA.serialize(obj2, {context: "crossprocess"});
            var dump2 = fmt.stringify(obj2);
    
            if (!arrEquals(blob, blob2) || dump !== dump2) {
                echo("unmatch");
                echo(dump2);
                fmt.print(blob2);
            }
       });

        echo();
    }
})();


