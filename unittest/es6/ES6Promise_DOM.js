var isWscript = typeof WScript !== 'undefined';
if (isWscript) {
  window = {};
  window.setTimeout = WScript.SetTimeout;
}

function echo(str) {
  if (!isWscript) {
    var p = document.createElement('p');
    p.textContent = str;
    var el = document.getElementById("testdiv");
    el.appendChild(p);
  } else {
    WScript.Echo(str);
  }
}

var promise = new Promise(
  function(resolve, reject) {
    window.setTimeout(
      function() {
        resolve('success');
      },
      0
    );
  }
);
var racePromise = Promise.race(
  [
    Promise.resolve(42),
    Promise.reject('fail'),
    promise
  ]
);
var allPromise = Promise.all(
  [
    Promise.resolve(42),
    Promise.resolve('not failure'),
    promise
  ]
);

promise.then(
  function(result) {
    echo('p1 success: ' + result);
    return racePromise;
  },
  function(err) {
    echo('p1 failure: ' + err);
  }
).then(function(result) {
    echo('p2 success: ' + result);
    return allPromise;
  }, function(err) {
    echo('p2 failure: ' + err);
  }
).then(function(result) {
    echo('p3 success: ' + result);
  }, function(err) {
    echo('p3 failure: ' + err);
  }
);

window.setTimeout(
  function() {
    if (isWscript) {
      WScript.Quit();
    }
  },
  100
);
