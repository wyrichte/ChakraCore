//Test for correct errors (baseline contains the message)
try {
    new Intl.Collator.supportedLocalesOf();
} catch (e) {
    WScript.Echo(e);
}
try {
    new Intl.NumberFormat.supportedLocalesOf();
} catch (e) {
    WScript.Echo(e);
}
try {
    new Intl.DateTimeFormat.supportedLocalesOf();
} catch (e) {
    WScript.Echo(e);
}
try {
    Intl.Collator.supportedLocalesOf(["en-US"], { localeMatcher: "incorrect" });
} catch (e) {
    WScript.Echo(e);
}
try {
    Intl.NumberFormat.supportedLocalesOf(["en-US"], { localeMatcher: "incorrect" });
} catch (e) {
    WScript.Echo(e);
}
try {
    Intl.DateTimeFormat.supportedLocalesOf(["en-US"], { localeMatcher: "incorrect" });
} catch (e) {
    WScript.Echo(e);
}
try{
    Intl.Collator.supportedLocalesOf(null);
} catch (e) {
    WScript.Echo(e);
}
try {
    Intl.NumberFormat.supportedLocalesOf(null);
} catch (e) {
    WScript.Echo(e);
}
try {
    Intl.DateTimeFormat.supportedLocalesOf(null);
} catch (e) {
    WScript.Echo(e);
}

//Test valid input
WScript.Echo(Intl.Collator.supportedLocalesOf(["en"]));
WScript.Echo(Intl.Collator.supportedLocalesOf(["en"], { localeMatcher: "lookup" }));
WScript.Echo(Intl.Collator.supportedLocalesOf(["en"], { localeMatcher: "best fit" }));
WScript.Echo(Intl.Collator.supportedLocalesOf().length);
WScript.Echo(Intl.Collator.supportedLocalesOf(undefined, { localeMatcher: "lookup" }).length);
WScript.Echo(Intl.Collator.supportedLocalesOf(undefined, { localeMatcher: "best fit" }).length);
WScript.Echo(Intl.Collator.supportedLocalesOf.call({}, ["en"]));
WScript.Echo(Intl.Collator.supportedLocalesOf.call({}, ["en"], { localeMatcher: "lookup" }));
WScript.Echo(Intl.Collator.supportedLocalesOf.call({}, ["en"], { localeMatcher: "best fit" }));
WScript.Echo(Intl.Collator.supportedLocalesOf.bind({})(["en"]));
WScript.Echo(Intl.Collator.supportedLocalesOf.bind({})(["en"], { localeMatcher: "lookup" }));
WScript.Echo(Intl.Collator.supportedLocalesOf.bind({})(["en"], { localeMatcher: "best fit" }));

//Duplicates for NumberFormat and DateTimeFormat
WScript.Echo(Intl.NumberFormat.supportedLocalesOf(["en"]));
WScript.Echo(Intl.NumberFormat.supportedLocalesOf(["en"], { localeMatcher: "lookup" }));
WScript.Echo(Intl.NumberFormat.supportedLocalesOf(["en"], { localeMatcher: "best fit" }));
WScript.Echo(Intl.NumberFormat.supportedLocalesOf().length);
WScript.Echo(Intl.NumberFormat.supportedLocalesOf(undefined, { localeMatcher: "lookup" }).length);
WScript.Echo(Intl.NumberFormat.supportedLocalesOf(undefined, { localeMatcher: "best fit" }).length);
WScript.Echo(Intl.NumberFormat.supportedLocalesOf.call({}, ["en"]));
WScript.Echo(Intl.NumberFormat.supportedLocalesOf.call({}, ["en"], { localeMatcher: "lookup" }));
WScript.Echo(Intl.NumberFormat.supportedLocalesOf.call({}, ["en"], { localeMatcher: "best fit" }));
WScript.Echo(Intl.NumberFormat.supportedLocalesOf.bind({})(["en"]));
WScript.Echo(Intl.NumberFormat.supportedLocalesOf.bind({})(["en"], { localeMatcher: "lookup" }));
WScript.Echo(Intl.NumberFormat.supportedLocalesOf.bind({})(["en"], { localeMatcher: "best fit" }));

WScript.Echo(Intl.DateTimeFormat.supportedLocalesOf(["en"]));
WScript.Echo(Intl.DateTimeFormat.supportedLocalesOf(["en"], { localeMatcher: "lookup" }));
WScript.Echo(Intl.DateTimeFormat.supportedLocalesOf(["en"], { localeMatcher: "best fit" }));
WScript.Echo(Intl.DateTimeFormat.supportedLocalesOf().length);
WScript.Echo(Intl.DateTimeFormat.supportedLocalesOf(undefined, { localeMatcher: "lookup" }).length);
WScript.Echo(Intl.DateTimeFormat.supportedLocalesOf(undefined, { localeMatcher: "best fit" }).length);
WScript.Echo(Intl.DateTimeFormat.supportedLocalesOf.call({}, ["en"]));
WScript.Echo(Intl.DateTimeFormat.supportedLocalesOf.call({}, ["en"], { localeMatcher: "lookup" }));
WScript.Echo(Intl.DateTimeFormat.supportedLocalesOf.call({}, ["en"], { localeMatcher: "best fit" }));
WScript.Echo(Intl.DateTimeFormat.supportedLocalesOf.bind({})(["en"]));
WScript.Echo(Intl.DateTimeFormat.supportedLocalesOf.bind({})(["en"], { localeMatcher: "lookup" }));
WScript.Echo(Intl.DateTimeFormat.supportedLocalesOf.bind({})(["en"], { localeMatcher: "best fit" }));
