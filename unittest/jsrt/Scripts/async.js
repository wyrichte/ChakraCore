injectedResult = 'result is pending';
Windows.Storage.KnownFolders.documentsLibrary.tryGetItemAsync('filedoesnotexist.txt').done(function (file) { 
  if (file == undefined) {
      injectedResult = 'does not exist';
  }
  else {
      injectedResult = 'exists';
  }
});
injectedResult.toString();