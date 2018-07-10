let o = {};
// this MSRC only occurs because our Intl implementation mutates `o`, while it should return a new object
// if our implementation changes in the future to not throw an error but instead not mutate o, that
// is still acceptable behavior.
let isSpecCompliant = false;

try {
    o = Intl.NumberFormat.apply(o);
    o = Intl.DateTimeFormat.apply(o);

    Intl.DateTimeFormat.prototype.formatToParts.apply(o);
    isSpecCompliant = true;
} catch (e) {
    if (e instanceof TypeError && /'this' is already initialized/.test(e.message)) {
        console.log("pass");
    } else {
        console.log(e.toString())
    }
} finally {
    if (isSpecCompliant) {
        console.log("pass");
    }
}
