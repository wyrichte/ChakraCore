function echo(m) { this.WScript ? WScript.Echo(m) : console.log(m); }

function oos() {
    oos();
}

try {
    oos();
} finally {
    try {
        oos();
    } catch (e) {
    } finally {
    }
}

//
// Win8: 772949
//      The inner finally cleared threadContext->OOS.thrownObject.
//
//      In chk build, outer finally asserts.
//      In fre build, outer finally gets a NULL thrownObject from shared OOS and sends
//          NULL upwards. ActiveScriptError::FillExcepInfo then AV as it expects a valid
//          thrownObject Var. NULL is not a valid Var.
//
