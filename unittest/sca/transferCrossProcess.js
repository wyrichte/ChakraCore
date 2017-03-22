WScript.LoadScriptFile("../../core/test/UnitTestFramework/UnitTestFramework.js");

function post(option) {
    var blob = SCA.serialize(
        option.obj, { context: option.context }, undefined, option.transferList);

    // jshost SCA stores __state__ property to hold transfer contents temporarily
    assert.areEqual(option.shouldTransfer, blob.hasOwnProperty('__state__'),
        "should have __state__ property if vars transferred");

    return SCA.deserialize(blob);
}

function test(option) {
    var a = new Int8Array([1, 9, 7]);

    // no transfer list
    assert.areEqual(
        '{"a":{"0":1,"1":9,"2":7},"b":133}',
        JSON.stringify(post({
            obj: {a: a, b: 133},
            context: option.context,
            transferList: [],
            shouldTransfer: false
    })));
    assert.areEqual('{"0":1,"1":9,"2":7}', JSON.stringify(a));

    // transfer/neuter
    assert.areEqual(
        '{"a":{"0":1,"1":9,"2":7},"b":133}',
        JSON.stringify(post({
            obj: {a: a, b: 133},
            context: option.context,
            transferList: [a.buffer],
            shouldTransfer: option.shouldTransfer
    })));
    assert.areEqual(
        option.shouldNeuter ? '{}' : '{"0":1,"1":9,"2":7}',
        JSON.stringify(a));

    if (option.shouldNeuter) {
        // error on already neutered
        assert.throws(() => post({
                obj: {a: a, b: 133},
                context: option.context,
                transferList: [a.buffer],
                shouldTransfer: false
        }));
    }
}

test({context: 'samethread', shouldTransfer: true, shouldNeuter: true});
test({context: 'crossthread', shouldTransfer: true, shouldNeuter: true});
test({context: 'crossprocess', shouldTransfer: false, shouldNeuter: true});
test({context: 'persist', shouldTransfer: false, shouldNeuter: false});
