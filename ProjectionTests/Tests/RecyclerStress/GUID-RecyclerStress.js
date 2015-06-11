if (typeof WScript !== 'undefined' && typeof WScript.LoadScriptFile !== 'undefined') { WScript.LoadScriptFile("..\\projectionsglue.js"); } 
(function () {
    runner.addTest({
        id: 1,
        desc: "Get/Set Id",
        pri: "0",
        test: function () {
            var myAnimal = new Animals.Animal(1);
            verify.noException(function () {
                myAnimal.id = "{00000000-0000-0000-0000-000000000123}";
                verify(myAnimal.id, "00000000-0000-0000-0000-000000000123", "Animal.id");
            }, "Marshaling in GUID");
        }
    });

    runner.addTest({
        id: 2,
        desc: "verifyMarshalGUID",
        pri: "0",
        test: function () {
            var myAnimal = new Animals.Animal(1);
            verify.noException(function () {
                var actual = myAnimal.verifyMarshalGUID("{60A36D60-6AC9-4266-B872-581295CE0C50}", "{60A36D60-6AC9-4266-B872-581295CE0C50}");
                verify(actual, "60a36d60-6ac9-4266-b872-581295ce0c50", "Verified GUID");
            }, "verifyMarshalGUID");
        }
    });

    runner.addTest({
        id: 3,
        desc: "verifyMarshalGUID",
        pri: "0",
        test: function () {
            var myAnimal = new Animals.Animal(1);
            verify.noException(function () {
                var actual = myAnimal.verifyMarshalGUID("{E55C507B-7C8F-4BDA-B576-A5AB98CF982B}", "E55C507B-7C8F-4BDA-B576-A5AB98CF982B");
                verify(actual, "e55c507b-7c8f-4bda-b576-a5ab98cf982b", "Verified GUID");
            }, "verifyMarshalGUID");
        }
    });

    runner.addTest({
        id: 4,
        desc: "verifyMarshalGUID",
        pri: "0",
        test: function () {
            var myAnimal = new Animals.Animal(1);
            verify.noException(function () {
                var actual = myAnimal.verifyMarshalGUID("{992810BA-C394-4429-B136-1BA8C5B54C5D}", "(992810BA-C394-4429-B136-1BA8C5B54C5D)");
                verify(actual, "992810ba-c394-4429-b136-1ba8c5b54c5d", "Verified GUID");
            }, "verifyMarshalGUID");
        }
    });

    runner.addTest({
        id: 5,
        desc: "verifyMarshalGUID",
        pri: "0",
        test: function () {
            var myAnimal = new Animals.Animal(1);
            verify.noException(function () {
                var actual = myAnimal.verifyMarshalGUID("{AADA726E-AFF5-4F56-9B03-C1EFE8497A0E}", "AADA726EAFF54F569B03C1EFE8497A0E");
                verify(actual, "aada726e-aff5-4f56-9b03-c1efe8497a0e", "Verified GUID");
            }, "verifyMarshalGUID");
        }
    });

    runner.addTest({
        id: 6,
        desc: "verifyMarshalGUID",
        pri: "0",
        test: function () {
            var myAnimal = new Animals.Animal(1);
            verify.noException(function () {
                var actual = myAnimal.verifyMarshalGUID("{0E7AB08B-F8AD-4187-80D9-CF2FB15EE7D7}", "{0xE7AB08B,0xF8AD,0x4187,{0x80,0xD9,0xCF,0x2F,0xB1,0x5E,0xE7,0xD7}}");
                verify(actual, "0e7ab08b-f8ad-4187-80d9-cf2fb15ee7d7", "Verified GUID");
            }, "verifyMarshalGUID");
        }
    });

    Loader42_FileName = 'Recycler Stress Selected Scenarios from Guid.js';
})();
if (typeof Run === 'function' && ((typeof window === 'undefined') || (typeof window.MSApp === 'undefined'))) { Run(); }
