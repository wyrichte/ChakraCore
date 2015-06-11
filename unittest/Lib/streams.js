WScript.StdErr.Write("write, ");
WScript.StdErr.WriteLine("writeline");

WScript.StdOut.Write("write, ");
WScript.StdOut.WriteLine("writeline");

while(!WScript.StdIn.EOF())
{
	WScript.Echo("STDIN: " + WScript.StdIn.ReadLine());
}
