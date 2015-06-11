/// <disable>JS2085,JS2038,JS2015,JS2064,JS2003,JS2043,JS2055,JS2087</disable>
// disable the unit testcases which not suitable to run in WWAHost/nightly
(function (){
  var testsToDisable={
      "array.js": ["fpa2"],
      "memoryleaks.js": [4],
      // media player playlist is n/a on phone
      "BPTTest.ObjID.Pri0test.js": [1, 2, 3, 7, 9, 11, 12, 13],
      
      // Windows.Devices.Sms is undefined
      // Windows.Storage.DownloadsFolder is undefined
      "UserScenarios.Activation.js": [1, 6],
      "UserScenarios.Activation.promise.js": [1, 6],

      // Windows.Storage.DownloadsFolder is undefined
      // createFileQueryWithOptions is not implemented
      "UserScenarios.DateTimeTest.js": [1, 2, 3],
      "UserScenarios.DateTimeTest.promise.js": [1, 2, 3],
      "UserScenarios.IOTest.js": [1, 2, 3, 4, 5, 6],
      "UserScenarios.IOTest.promise.js": [1, 2, 3, 4, 5, 6],

      // Windows namespace is not enumberable
      "UserScenarios.Namespaces.js": [17, 18, 19, 28, 29, 30],
      "WindowsNamespaceEnumerability.js": [3],
      "WindowsNamespaceEnumerability.winBlue.js": [3],

      // Windows.Data.Xml.Dom.XmlDocument.loadFromUriAsync seems not work, also phone can't access local domain resource
      "UserScenarios.XmlReader.js": [2],
      "UserScenarios.XmlReader.promise.js": [2],      
  }
  
  var currentTestFile;
  runner.subscribe('start', function(fileName, priority){
      currentTestFile = fileName;
  });
  
  runner.subscribe('testStart', function(t){
      if(typeof t.id !== "undefined"){
        if( typeof currentTestFile !== 'undefined'){
          disables=testsToDisable[currentTestFile];
        } else if(typeof Loader42_FileName !== 'undefined'){
          disables=testsToDisable[Loader42_FileName];
        }
        
        if (Array.isArray(disables) && disables.some(function (e) { return e == t.id || e == t.desc })) {
          t.test = function(){
            logger.comment("this test is skipped");
          };
          return;
        }    
    }
  }); 

})();