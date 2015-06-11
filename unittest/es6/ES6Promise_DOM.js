function echo(str) {
    if (typeof WScript === 'undefined') {
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
promise.then(
	function(result) {
		echo('p1 success: ' + result);
	},
	function(err) {
		echo('p1 failure: ' + err);
	}
);

Promise.race(
	[
		Promise.resolve(42),
		Promise.reject('fail'),
		promise
	]
).then(
	function(result) {
		echo('p2 success: ' + result);
	}
).catch(
	function(err) {
		echo('p2 failure: ' + err);
	}
);

Promise.all(
	[
		Promise.resolve(42),
		Promise.resolve('not failure'),
		promise
	]
).then(
	function(result) {
		echo('p3 success: ' + result);
	}
).catch(
	function(err) {
		echo('p3 failure: ' + err);
	}
);

window.setTimeout(
	function() {
		WScript.Quit();
	},
	100
);
