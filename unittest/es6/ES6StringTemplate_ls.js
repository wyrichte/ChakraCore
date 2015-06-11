` `;

function /**target:get10**/get10() {
 return 10;
}

var str = `Got ${get10/**gd:get10**/()} ${get10()} string`;
var str = get10`Got ${get10/**gd:get10**/()} ${get10()} string`;
