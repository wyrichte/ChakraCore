for(var i = 0; i < 100; ++i)
{
	if(i == 50)
	{
		WScript.Echo(i, i*i, i*i*i+100);
	}
	else
	{
		WScript.Echo(i, i*i, i*i*i);
	}
}