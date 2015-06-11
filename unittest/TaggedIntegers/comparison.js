// Tagged Integers Test Plan, testcase #1
// 
// Tests integer comparisons, with special attention to values interesting to
// tagged integers.


function check(cond,str)
{
	if(!cond)
	{
		WScript.Echo("FAIL: " + str);
	}
}


// BEGIN INTEGER DEFINITIONS
var n = 573;
var m = 572;
var t = new Object;

t.zero1 = 0;
t.zero2 = 0x0;
t.zero3 = new Number(0);
t.zero4 = m - m;
//WScript.Echo("--- 0 ---");
//WScript.Echo(t.zero1);
//WScript.Echo(t.zero2);
//WScript.Echo(t.zero3);
//WScript.Echo(t.zero4);

t.one1 = 1
t.one2 = 0x1;
t.one3 = new Number(1);
t.one4 = n - m;
//WScript.Echo("--- 1 ---");
//WScript.Echo(t.one1);
//WScript.Echo(t.one2);
//WScript.Echo(t.one3);
//WScript.Echo(t.one4);

t.two1 = 2;
t.two2 = 0x2;
t.two3 = new Number(2);
t.two4 = 2*(n-m);
//WScript.Echo("--- 2 ---");
//WScript.Echo(t.two1);
//WScript.Echo(t.two2);
//WScript.Echo(t.two3);
//WScript.Echo(t.two4);

t.negone1 = -1
t.negone2 = -0x1;
t.negone3 = -new Number(1);
t.negone4 = m-n;
//WScript.Echo("--- -1 ---");
//WScript.Echo(t.negone1);
//WScript.Echo(t.negone2);
//WScript.Echo(t.negone3);
//WScript.Echo(t.negone4);

t.negtwo1 = -2;
t.negtwo2 = -0x2;
t.negtwo3 = -new Number(2);
t.negtwo4 = 2*(m-n);
//WScript.Echo("--- -2 ---");
//WScript.Echo(t.negtwo1);
//WScript.Echo(t.negtwo2);
//WScript.Echo(t.negtwo3);
//WScript.Echo(t.negtwo4);

t.tagmax1 = 536870911;
t.tagmax2 = 0x1fffffff;
t.tagmax3 = new Number(536870911);
t.tagmax4 = 936947*n+280;
//WScript.Echo("--- tag_max ---");
//WScript.Echo(t.tagmax1);
//WScript.Echo(t.tagmax2);
//WScript.Echo(t.tagmax3);
//WScript.Echo(t.tagmax4);

t.tagmin1 = -536870912;
t.tagmin2 = -0x20000000;
t.tagmin3 = new Number(-536870912);
t.tagmin4 = -(936947*n+280)-1;
//WScript.Echo("--- tag_min ---");
//WScript.Echo(t.tagmin1);
//WScript.Echo(t.tagmin2);
//WScript.Echo(t.tagmin3);
//WScript.Echo(t.tagmin4);

t.tagmaxminusone1 = 536870910;
t.tagmaxminusone2 = 0x1ffffffe;
t.tagmaxminusone3 = new Number(536870910);
t.tagmaxminusone4 = 936947*n+280-1;
//WScript.Echo("--- tag_max-1 ---");
//WScript.Echo(t.tagmaxminusone1);
//WScript.Echo(t.tagmaxminusone2);
//WScript.Echo(t.tagmaxminusone3);
//WScript.Echo(t.tagmaxminusone4);

t.tagminplusone1 = -536870911;
t.tagminplusone2 = -0x1fffffff;
t.tagminplusone3 = new Number(-536870911);
t.tagminplusone4 = -(936947*n+280);
//WScript.Echo("--- tag_min+1 ---");
//WScript.Echo(t.tagminplusone1);
//WScript.Echo(t.tagminplusone2);
//WScript.Echo(t.tagminplusone3);
//WScript.Echo(t.tagminplusone4);

t.uintmax1 = 4294967295;
t.uintmax2 = 0xffffffff;
t.uintmax3 = new Number(4294967295);
t.uintmax4 = 7495579*n+528;
//WScript.Echo("--- uint_max ---");
//WScript.Echo(t.uintmax1);
//WScript.Echo(t.uintmax2);
//WScript.Echo(t.uintmax3);
//WScript.Echo(t.uintmax4);

t.uintmaxminusone1 = 4294967294;
t.uintmaxminusone2 = 0xfffffffe;
t.uintmaxminusone3 = new Number(4294967294);
t.uintmaxminusone4 = 7495579*n+528-1;
//WScript.Echo("--- uint_max-1 ---");
//WScript.Echo(t.uintmaxminusone1);
//WScript.Echo(t.uintmaxminusone2);
//WScript.Echo(t.uintmaxminusone3);
//WScript.Echo(t.uintmaxminusone4);

t.intmax1 = 2147483647;
t.intmax2 = 0x7fffffff;
t.intmax3 = new Number(2147483647);
t.intmax4 = 2147483074+n;
//WScript.Echo("--- int_max ---");
//WScript.Echo(t.intmax1);
//WScript.Echo(t.intmax2);
//WScript.Echo(t.intmax3);
//WScript.Echo(t.intmax4);

t.intmaxminusone1 = 2147483646;
t.intmaxminusone2 = 0x7ffffffe;
t.intmaxminusone3 = new Number(2147483646);
t.intmaxminusone4 = 2147483073+n;
//WScript.Echo("--- int_max-1 ---");
//WScript.Echo(t.intmaxminusone1);
//WScript.Echo(t.intmaxminusone2);
//WScript.Echo(t.intmaxminusone3);
//WScript.Echo(t.intmaxminusone4);

t.intmin1 = -2147483648;
t.intmin2 = -0x80000000;
t.intmin3 = new Number(-2147483648);
t.intmin4 = -2147483075-n;
//WScript.Echo("--- int_min ---");
//WScript.Echo(t.intmin1);
//WScript.Echo(t.intmin2);
//WScript.Echo(t.intmin3);
//WScript.Echo(t.intmin4);

t.intminplusone1 = -2147483647;
t.intminplusone2 = -0x7fffffff;
t.intminplusone3 = new Number(-2147483647);
t.intminplusone4 = -2147483074-n;
//WScript.Echo("--- int_min+1 ---");
//WScript.Echo(t.intminplusone1);
//WScript.Echo(t.intminplusone2);
//WScript.Echo(t.intminplusone3);
//WScript.Echo(t.intminplusone4);

//
// These values are in increasing numeric value.  It's important to keep
// that ordering.
//
var testvals = [ "intmin",
		 "intminplusone",
		 "tagmin",
		 "tagminplusone",
		 "negtwo",
		 "negone",
		 "zero",
		 "one",
		 "two",
		 "tagmaxminusone",
		 "tagmax",
		 "intmaxminusone",
		 "intmax",
		 "uintmaxminusone",
		 "uintmax"
		];


// END DEFINITIONS



// Test for equality
function check_equality()
{
	for(var idx = 0; idx < testvals.length; ++idx)
	{
		//WScript.Echo("Checking equality for... " + testvals[idx]);
		for(var i = 1; i <= 4; ++i)
		{
			for(var j = 1; j <= 4; ++j)
			{
				var l1 = testvals[idx]+i;
				var l2 = testvals[idx]+j;
	
				var result = false;
	
				// work around bug#25
				if(t[l1] == t[l2])
				{
					result = true;
				}
				check(result, t[l1] + " == " + t[l2]);
			}
		}
	}
}


// Test for inequality
function check_inequality()
{
	for(var idx1 = 0; idx1 < testvals.length; ++idx1)
	{

		for(var idx2 = 0; idx2 < testvals.length; ++idx2)
		{
			if(idx1 == idx2)
				continue;

			//WScript.Echo("Checking inequality for... " + testvals[idx1] + " != " + testvals[idx2]);
			for(var i = 1; i <= 4; ++i)
			{
				for(var j = 1; j <= 4; ++j)
				{
					var l1 = testvals[idx1]+i;
					var l2 = testvals[idx2]+j;

					var result = false;
		
					// work around bug#25
					if(t[l1] != t[l2])
					{
						result = true;
					}
					check(result, t[l1] + " != " + t[l2]);					
				}
			}
		}
	}
}

// Test for greater/less than
function check_greaterless()
{
	for(var idx1 = 0; idx1 < testvals.length; ++idx1)
	{

		for(var idx2 = 0; idx2 < testvals.length; ++idx2)
		{
			if(idx1 == idx2)
				continue;

			//WScript.Echo("Checking less for... " + testvals[idx1] + " != " + testvals[idx2]);

			for(var i = 1; i <= 4; ++i)
			{
				for(var j = 1; j <= 4; ++j)
				{
					var l1 = testvals[idx1]+i;
					var l2 = testvals[idx2]+j;
					var result = false;

					var str = "ERROR";
		
					if(idx1 > idx2)
					{
						str = " > ";

						// work around bug#25
						if(t[l1] > t[l2])
						{
							result = true;
						}
					}
					else if(idx1 < idx2)
					{
						str = " < ";
												// work around bug#25
						if(t[l1] < t[l2])
						{
							result = true;
						}
					}
					else
					{
						WScript.Echo("should never get here!");
						result = false;
					}

					check(result, t[l1] + str + t[l2]);					
				}
			}
		}
	}
}


// Test for greaterequals/lessequals
function check_greaterlessequals()
{
	for(var idx1 = 0; idx1 < testvals.length; ++idx1)
	{

		for(var idx2 = 0; idx2 < testvals.length; ++idx2)
		{
			//WScript.Echo("Checking less for... " + testvals[idx1] + " != " + testvals[idx2]);

			for(var i = 1; i <= 4; ++i)
			{
				for(var j = 1; j <= 4; ++j)
				{
					var l1 = testvals[idx1]+i;
					var l2 = testvals[idx2]+j;
					var result = false;

					var str = "ERROR";
		
					if(idx1 > idx2)
					{
						str = " >= ";

						// work around bug#25
						if(t[l1] >= t[l2])
						{
							result = true;
						}
					}
					else if(idx1 < idx2)
					{
						str = " <= ";
												// work around bug#25
						if(t[l1] <= t[l2])
						{
							result = true;
						}
					}
					else if(idx1 == idx2)
					{
						if(i >= j)
						{
							if(t[l1] >= t[l2])
							{
								result = true;
							}
						}
						else
						{
							if(t[l1] <= t[l2])
							{
								result = true;
							}
						}					
					}

					check(result, t[l1] + str + t[l2]);					
				}
			}
		}
	}
}

check_equality();
check_inequality();
check_greaterless();
check_greaterlessequals();

WScript.Echo("done");