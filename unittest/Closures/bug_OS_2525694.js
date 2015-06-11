(function () {
  with ({}) {
    try {
      arguments(z) = w; // shouldn't crash
    } catch (e) {
    }
  }
})();

WScript.Echo("passed");
