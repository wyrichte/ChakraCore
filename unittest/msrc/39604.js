function f() {
    {
        let i;
        function g() {i;}

        try {
            throw 1;
        } catch ({e = eval('i')}) {}
    }
}

function g() {
    {
        let i;
        function g() {i;}

        try {
            throw 1;
        } catch ({e = (()=>{ eval('i'); })() }) {}
    }
}

f();
g();
console.log("pass");
