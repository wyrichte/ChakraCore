_map = [].map;

function foo() {
    [1].map(function() {
        print(_map.caller === null ?
            "pass" : "fail: caller: " + _map.caller);
    });
}

foo();
