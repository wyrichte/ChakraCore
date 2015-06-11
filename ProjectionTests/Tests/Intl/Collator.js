if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {
    // Tests around ABI Activation

    //Test valid input options and their output.
    runner.addTest({
        id: 1,
        desc: 'Test Valid Options',
        pri: '0',
        test: function () {
            try {
                verify(new Intl.Collator("en-US").compare("a", "A"), -1, "Comparing with default options.");
                verify(new Intl.Collator("en-US", { sensitivity: "variant" }).compare("a", "A"), -1, "Comparing with variant sensitivity.");
                verify(new Intl.Collator("en-US", { sensitivity: "variant" }).compare("\u00C0", "A"), 1, "Comparing with variant sensitivity.");
                verify(new Intl.Collator("en-US", { sensitivity: "variant" }).compare("a", "b"), -1, "Comparing with variant sensitivity.");
                verify(new Intl.Collator("en-US", { sensitivity: "case" }).compare("a", "A"), -1, "Comparing with case sensitivity.");
                verify(new Intl.Collator("en-US", { sensitivity: "case" }).compare("a", "b"), -1, "Comparing with case sensitivity.");
                verify(new Intl.Collator("en-US", { sensitivity: "case" }).compare("\u00C0", "A"), 0, "Comparing with case sensitivity.");
                verify(new Intl.Collator("en-US", { sensitivity: "accent" }).compare("\u00C0", "A"), 1, "Comparing with accent sensitivity.");
                verify(new Intl.Collator("en-US", { sensitivity: "accent" }).compare("a", "A"), 0, "Comparing with accent sensitivity.");
                verify(new Intl.Collator("en-US", { sensitivity: "accent" }).compare("a", "b"), -1, "Comparing with accent sensitivity.");
                verify(new Intl.Collator("en-US", { sensitivity: "base" }).compare("a", "A"), 0, "Comparing with base sensitivity.");
                verify(new Intl.Collator("en-US", { sensitivity: "base" }).compare("\u00C0", "A"), 0, "Comparing with base sensitivity.");
                verify(new Intl.Collator("en-US", { sensitivity: "base" }).compare("a", "b"), -1, "Comparing with base sensitivity.");
                verify(new Intl.Collator("de-DE", { collation: "phonebk" }).compare("äb", "ada"), 1, "Comparing with collation option of phonebk.");
                verify(new Intl.Collator("de-DE-u-co-phonebk", {}).compare("\u00e4b", "ada"), 1, "Comparing with collation unciode extension phonebk.");
                verify(new Intl.Collator("de-DE", {}).compare("\u00e4b", "ada"), -1, "Comparing without collation option of phonebk.");
                verify(new Intl.Collator("de-DE", { numeric: true }).compare("21", "100"), -1, "Comparing with numeric option set to true.");
                verify(new Intl.Collator("de-DE-u-kn-true", {}).compare("21", "100"), -1, "Comparing with numeric unicode extension set to true.");
                verify(new Intl.Collator("de-DE", { numeric: false }).compare("21", "100"), 1, "Comparing with numeric option set to false.");
                verify(new Intl.Collator("de-DE-u-kn-false", {}).compare("21", "100"), 1, "Comparing with numeric unicode extension set to false.");
                verify(new Intl.Collator("de-DE-u-kn-true-co-phonebk", {}).compare("21", "100"), -1, "Comparing with collation set to phonebk and numeric set to true.");
                verify(new Intl.Collator("de-DE-u-kn-true-co-phonebk", {}).compare("\u00e4b", "ada"), 1, "Comparing with collation set to phonebk and numeric set to true.");
                verify(new Intl.Collator("de-DE-u-kn-false-co-phonebk", {}).compare("21", "100"), 1, "Comparing with collation set to phonebk and numeric set to false.");
                verify(new Intl.Collator("de-DE-u-kn-false-co-phonebk", {}).compare("\u00e4b", "ada"), 1, "Comparing with collation set to phonebk and numeric set to false.");
                verify(new Intl.Collator("en-US", { ignorePunctuation: true }).compare("aa", "a!a"), 0, "Comparing with ignore punctuation set to true.");
                verify(new Intl.Collator("en-US", { ignorePunctuation: false }).compare("aa", "a!a"), 1, "Comparing with ignore punctuation set to true.");
            }
            catch (e) {
                fail("Exception wasn't expected. Message: " + e);
            }
        }
    });

    //Test valid input options and their output.
    runner.addTest({
        id: 2,
        desc: 'Test Invalid Options',
        pri: '0',
        test: function () {

            function verifyCollatorException(locale, options, expectingInvalidOption, validValuesStr) {
                try {
                    //Since minute and second aren't supported alone; doing this to prevent that exception.
                    new Intl.Collator(locale, options);
                    fail("Exception was expected. Option: " + expectingInvalidOption + "; options passed in: " + JSON.stringify(options));
                }
                catch (e) {
                    if (!(e instanceof RangeError || e instanceof TypeError)) {
                        fail("Incorrect exception was thrown.");
                    }
                    verify(e.message.indexOf(validValuesStr) !== -1, true, "Exception didn't have the correct valid values when testing option:" + expectingInvalidOption + ".\nMessage: " + e.message + "\nSearched For:" + validValuesStr);
                }
            }

            try {
                verifyCollatorException("en-US-u-kf-invalid", {}, "caseFirst", "['upper', 'lower', 'false']");
                verifyCollatorException("en-US", { caseFirst: "invalid" }, "caseFirst", "['upper', 'lower', 'false']");

                verify(new Intl.Collator("en-US", { numeric: "blah" }).resolvedOptions().numeric, true, "Testing invalid numeric option.");
                verify(new Intl.Collator("en-US-u-kn-blah", {}).resolvedOptions().numeric, false, "Testing invalid numeric option.");
                verify(new Intl.Collator("en-US", { ignorePunctuation: "blah" }).resolvedOptions().ignorePunctuation, true, "Testing invalid ignorePunctuation option.");
                verify(new Intl.Collator("en-US", { collation: "blah" }).resolvedOptions().collation, "default", "Testing invalid collation option.");
                verify(new Intl.Collator("en-US-u-co-blah", {}).resolvedOptions().collation, "default", "Testing invalid colation option.");
            }
            catch (e) {
                fail("Exception wasn't expected.");
            }
        }
    });


    Loader42_FileName = "Intl Collator Tests"
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }