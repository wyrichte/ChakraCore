// Bug : 296426, validation of script profiler with Intl code.

function f1()
{
    var intl = Intl.Collator("Hi");
    intl.compare("à¤ˆ", "à¤“");
}

WScript.StartProfiling(f1);
