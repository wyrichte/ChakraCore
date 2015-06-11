
function test1() {
    var k = 10;
    k += testInternal();
};

function testInternal() {
    return 10;                 
}

function test2() {
	function test2_sub() {
		callback();
	}
	test2_sub();
}
