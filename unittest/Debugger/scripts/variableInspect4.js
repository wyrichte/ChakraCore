function foobaz(a, b) {
    arguments[5] = "extra arg";
    return 10;
}

function foobaz1(c, d) {
    "use strict"
    return 10;
}

foobaz(2);
foobaz(2, 100);
foobaz(2, new Date, 4);
foobaz("hi", undefined, 4, "hello");

foobaz1(2);
foobaz1(12, 100);
foobaz1(12, new Date, 4);
foobaz1("hi", undefined, 4, "hello");
