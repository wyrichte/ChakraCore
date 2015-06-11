
function SetArrayIndex(a, i)
{
    a[i] = "blah";
}

function test_obj_proto(proto)
{
    var obj = Object.create(proto);
    //var obj = {};
    obj[0] = 0;
    SetArrayIndex(obj, 0);
    SetArrayIndex(obj, 1);
    WScript.Echo(obj[1]);
}
