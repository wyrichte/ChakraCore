echo(new RegExp("", "mig").toString());
echo(new RegExp("/").toString());
echo(new RegExp("\\/").toString());
echo(new RegExp("\\\\/").toString());
echo(new RegExp("\\\\\\//").toString());
echo(new RegExp("\n").toString());
echo(new RegExp("\\\n").toString());

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

var web;
function echo(o) {
    if (web)
        document.write(o + "<br/>");
    else {
        try {
            WScript.Echo("" + o);
        } catch (ex) {
            print("" + o);
        }
    }
}
