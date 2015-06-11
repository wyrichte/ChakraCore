if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); }
(function () {

    //Test valid input options and their output.
    runner.addTest({
        id: 1,
        desc: 'Test Valid Options',
        pri: '0',
        test: function () {
            try {
                verify(new Intl.NumberFormat("en-US", { minimumSignificantDigits: 2, maximumSignificantDigits: 2 }).format(2.131), "2.1", "Formatting number with significant digits.");
                verify(new Intl.NumberFormat("en-US", { minimumSignificantDigits: 2, maximumSignificantDigits: 3 }).format(2.1), "2.1", "Formatting number with significant digits.");
                verify(new Intl.NumberFormat("en-US", { minimumSignificantDigits: 3, maximumSignificantDigits: 3 }).format(2.1), "2.10", "Formatting number with significant digits.");
                verify(new Intl.NumberFormat("en-US", { minimumSignificantDigits: 3, maximumSignificantDigits: 3 }).format(123.1), "123", "Formatting number with significant digits.");
                verify(new Intl.NumberFormat("en-US", { minimumSignificantDigits: 3, maximumSignificantDigits: 3, style: "decimal" }).format(123.1), "123", "Formatting number with significant digits.");
// Disabled because of change in Windows Globalization
// Reenable as part of DEVDIV:989434
/*
                verify(new Intl.NumberFormat("en-US", { minimumSignificantDigits: 3, maximumSignificantDigits: 3, style: "percent" }).format(123.1), "12,300 %", "Formatting number with significant digits.");
                verify(new Intl.NumberFormat("en-US", { minimumSignificantDigits: 3, maximumSignificantDigits: 3, style: "percent", useGrouping: false }).format(123.1), "12300 %", "Formatting number with significant digits.");
*/

                verify(new Intl.NumberFormat("en-US", { minimumSignificantDigits: 3, maximumSignificantDigits: 3, style: "currency", currency: "USD" }).format(123.1), "$123", "Formatting number with significant digits.");
// Disabled because of change in Windows Globalization
// Reenable as part of DEVDIV:989434
/*
                verify(new Intl.NumberFormat("en-US", { minimumSignificantDigits: 3, maximumSignificantDigits: 3, style: "currency", currency: "USD", currencyDisplay: "code" }).format(123.1), "USD 123", "Formatting number with significant digits.");
*/

                verify(new Intl.NumberFormat("en-US", { minimumSignificantDigits: 3, maximumSignificantDigits: 3, style: "currency", currency: "USD", currencyDisplay: "symbol" }).format(123.1), "$123", "Formatting number with significant digits.");
// Disabled because of change in Windows Globalization
// Reenable as part of DEVDIV:989434
/*
                verify(new Intl.NumberFormat("en-US", { minimumSignificantDigits: 3, maximumSignificantDigits: 3, style: "currency", currency: "USD", currencyDisplay: "name" }).format(123.1), "USD 123", "Formatting number with significant digits.");
*/
                verify(new Intl.NumberFormat("en-US", { minimumSignificantDigits: 3, maximumSignificantDigits: 3, minimumIntegerDigits: 5, minimumFractionDigits: 5, maximumFractionDigits: 5 }).format(123.1), "123", "Formatting number with significant digits.");
                verify(new Intl.NumberFormat("en-US", { minimumIntegerDigits: 5, minimumFractionDigits: 5, maximumFractionDigits: 5 }).format(123.1), "00,123.10000", "Formatting number with significant digits.");
                verify(new Intl.NumberFormat("en-US", { minimumIntegerDigits: 1, minimumFractionDigits: 1, maximumFractionDigits: 3 }).format(123.14444), "123.144", "Formatting number with significant digits.");
            }
            catch (e) {
                fail("Exception wasn't expected.");
            }
        }
    });

    //Test valid input options and their output.
    runner.addTest({
        id: 2,
        desc: 'Test Invalid Options',
        pri: '0',
        test: function () {
            
            function verifyNFException(locale, options, expectingInvalidOption, validValuesStr) {
                try {
                    //Since minute and second aren't supported alone; doing this to prevent that exception.
                    new Intl.NumberFormat(locale, options);
                    fail("Exception was expected. Option: " + expectingInvalidOption + "; options passed in: " + JSON.stringify(options));
                }
                catch (e) {
                    if (!(e instanceof RangeError || e instanceof TypeError)) {
                        fail("Incorrect exception was thrown.");
                    }
                    verify(e.message.indexOf(validValuesStr) !== -1, true,  "Exception didn't have the correct valid values when testing option:" + expectingInvalidOption + ".\nMessage: " + e.message + "\nSearched For:" + validValuesStr);
                }
            }

            try {
                verifyNFException("en-US", { minimumSignificantDigits: -1 }, "minimumSignificantDigits", "[1 - 21]");
                verifyNFException("en-US", { maximumSignificantDigits: -1 }, "maximumSignificantDigits", "[1 - 21]");
                verifyNFException("en-US", { minimumFractionDigits: -1 }, "minimumFractionDigits", "[0 - 20]");
                verifyNFException("en-US", { maximumFractionDigits: -1 }, "maximumFractionDigits", "[0 - 20]");
                verifyNFException("en-US", { minimumIntegerDigits: -1 }, "minimumIntegerDigits", "[1 - 21]");

                verifyNFException("en-US", { minimumSignificantDigits: 22 }, "minimumSignificantDigits", "[1 - 21]");
                verifyNFException("en-US", { maximumSignificantDigits: 22 }, "maximumSignificantDigits", "[1 - 21]");
                verifyNFException("en-US", { minimumFractionDigits: 21 }, "minimumFractionDigits", "[0 - 20]");
                verifyNFException("en-US", { maximumFractionDigits: 21 }, "maximumFractionDigits", "[0 - 20]");
                verifyNFException("en-US", { minimumIntegerDigits: 22 }, "minimumIntegerDigits", "[1 - 21]");

                verifyNFException("en-US", { minimumSignificantDigits: 5, maximumSignificantDigits: 1 }, "maximumSignificantDigits", "[5 - 21]");
                verifyNFException("en-US", { minimumFractionDigits: 5, maximumFractionDigits: 1 }, "maximumFractionDigits", "[5 - 20]");

                verifyNFException("en-US", { style: "invalid" }, "style", "['decimal', 'percent', 'currency']");
                verifyNFException("en-US", { style: "currency" }, "style", "Currency code was not specified");
                verifyNFException("en-US", { style: "currency", currency: 5 }, "currency", "Currency code '5' is invalid");
                verifyNFException("en-US", { style: "currency", currency: "USD", currencyDisplay: "invalid" }, "currencyDisplay", "['code', 'symbol', 'name']");
            }
            catch (e) {
                fail("Exception wasn't expected.");
            }
        }
    });


    Loader42_FileName = "Intl NumberFormat Tests"
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
