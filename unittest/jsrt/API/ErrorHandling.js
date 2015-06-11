function throwAtHost() {
  throw "throwing";
}

function callHostWithTryCatch() {
  try {
    callHost();
  }
  catch (error) {
    return true;
  }
  
  return false;
}

function callHostWithNoTryCatch() {
  callHost();
  
  return false;
}