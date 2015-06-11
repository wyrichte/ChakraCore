function echo(str) {
    self.postMessage(str);
}

function test() {
    var promise = new Promise( 
        function(resolve, reject) {
            setTimeout(
                function() {
                    resolve('success');
                },
                0
            );
        }
    );
    promise.then(
        function(result) {
            echo('success: ' + result);
            self.close();
        },
        function(err) {
            echo('failure: ' + err);
        }
    );
}

self.addEventListener('message', function(event) { test(); }, false);
