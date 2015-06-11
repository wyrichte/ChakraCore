//
// Parameter index test
//
function ParamIndexTest() {
	function fnc(aa, bb, cc, dd) {}
	fnc(fnc(),| fnc(a,b|)|); // 111
	fnc(|
		"aaa"|,
		 "bbb"|
		 ,|); // 0012
 	fnc(0 /* , */|); // 0
	fnc(| 0| ,| 1|,| 2|,| 3|); // 00112233
	fnc(|0|,|1|,|2|,|3|); // 00112233
	fnc(|  0| /* , */| ,|  1| ,|  2| ,|3| /* aa */ ); // 0001122333
	fnc(  0 /* , */ ,  1 ,  2 ,3 /* aa */| ); // 3
	fnc(  0  ,  1 ,  2 ,|3); // 3
	fnc(  0  ,  1 ,  2|,3); // 3
	fnc( 1 ,| ); // 1
	fnc(|); // 0
	fnc(1|); // 0
	fnc(1|,); // 0
	fnc(1,|); // 1
	fnc( 1 , 2,|); // 2
	fnc(0 ,  1| ,  2 ,|); // 13
	fnc(0 ,  1 ,  2 ,|); // 3
	fnc(  0  ,  1 ,  2 ,|); // 3
	fnc(  0  ,  1 ,  2 ,| ); // 3
	fnc(fnc(),| fnc(a,b)); // 1
}
