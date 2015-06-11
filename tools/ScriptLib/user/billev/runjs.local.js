var szInput = "shirt.txt";
var szOutput = "shirt.html";

var disciplineColors = ["\"red\"", "\"green\"", "\"blue\"", "\"purple\"", "\"brown\"", "\"orange\""];

var ixName, ixEmail, ixYears, ixIsLead, ixDiscipline, ixGender, ixCareer, ixTraxEmail;

//******************************************************************************
// This parses the header, to determine which columns contain fields of interest.
function ParseTeamHeader(line)
{
	var info = line.split("\t");
	for (var i=0; i<info.length; i++)
	{
		switch (info[i])
		{
		case "Full Name":
			ixName = i;
			break;
		case "TraxEmail":
			ixTraxEmail = i;
			break;
		case "Email":
			ixEmail = i;
			break;
		case "Years":
			ixYears = i;
			break;
		case "IsLead":
			ixIsLead = i;
			break;
		case "Discipline":
			ixDiscipline = i;
			break;
		case "Gender":
			ixGender = i;
			break;
		case "Career":
			ixCareer = i;
			break;
		}
	}

}

//******************************************************************************
// Parse a line into a team member record.
function teamMemberCtor(line)
{
	var info = line.split("\t");
	var t;

	this.Name = info[ixName];
	this.Email = info[ixEmail];
	if (this.Email == "#N/A")
		this.Email = info[ixTraxEmail];
	this.Years = info[ixYears];
	this.IsLead = info[ixIsLead];
	this.Discipline = info[ixDiscipline];
	t = info[ixGender];
	this.Gender = 0;
	if (t == "F")
		this.Gender = 1;
	t = info[ixCareer];
	this.Career = 0;
	if (t == "Yes")
		this.Career = 1;
}

//******************************************************************************
// Read the tab-delimited file of team member info.
function ReadTeamMembers(teamHash)
{
    var teamFile = FSOOpenTextFile(szInput, FSOForReading);
	var line = teamFile.ReadLine();
	ParseTeamHeader(line);
    
    while (!teamFile.AtEndOfStream) 
	{
        line = teamFile.ReadLine();
		var member = new teamMemberCtor(line);
		teamHash.add(member.Email, member);
    }
    teamFile.Close();


}

//******************************************************************************
// Write one team member's alias, with appropriate decoration.
function WriteMember(member, htmlFile)
{
	var bItalic = false;
	var bUnder = false;
	
	// Size based on time
	var fontSize = "";
	if (member.Years > 6)
	{
		fontSize = " size=+2";
	}
	else
	if (member.Years > 4)
	{
		fontSize = " size=+1";
	}
	else
	if (member.Years < 2)
	{
		fontSize = " size=-1";
	}
	
    if (member.Years > 7) // Pioneer
		bItalic = true;
	if (member.Career)
		bUnder = true;

	var face = "";
	if (!member.Gender)
		face = " face=\"Arial Rounded MT Bold\"";
	else
		face = " face=\"Maiandra GD\"";

	// Bold based on lead status
	htmlFile.Write(" ");
	if (member.IsLead > 0) 
		htmlFile.Write("<STRONG>");
	if (bItalic)
		htmlFile.Write("<I>");
	if (bUnder)
		htmlFile.Write("<u>");

	// Color based on discipline
	htmlFile.Write("<font color=" + disciplineColors[member.Discipline] + fontSize + face + ">");

	htmlFile.Write(member.Email);

	htmlFile.Write("</font>");

	if (bUnder)
		htmlFile.Write("</u>");
	if (bItalic)
		htmlFile.Write("</I>");
	if (member.IsLead > 0) 
		htmlFile.Write("</STRONG>");

	htmlFile.WriteLine(" "); // slight spacing between names

}

/****************************************************************************/
/* Read data about CLR team members, and produce HTML file for tee-shirt.
*/
function teeShirt()
{
    logCall(LogTestSummary, LogInfo10, "makeFailureSummary", arguments, "{");

    // If under a debugger, stop here so that we have a chance to put BPs in the file.
    debugger;

    var htmlFile = FSOOpenTextFile(szOutput, 2, true); 
    htmlFile.WriteLine("<HTML>"); 
    htmlFile.WriteLine("<BODY>"); 
    htmlFile.WriteLine("<H2> CLR Tee Shirt v 3.0 (" + new Date() + ") </H2>"); 

    // Hash of all errors. Input is the test guid; output is HTML.
    var teamHash = _newHashtable(); // hash of FailTestGroup objects

	ReadTeamMembers(teamHash);
    
	var list = (new VBArray(teamHash.Items())).toArray();   // Get the keys.


	htmlFile.WriteLine("<P align=\"justify\">");
	for (var i = 0; i < list.length; i++) 
	{
		// Add each test failure as an instance to the group.
		var teamMember = list[i];

		WriteMember(teamMember, htmlFile);
	}
	htmlFile.WriteLine("</P>");

	htmlFile.WriteLine("</BODY>");
	htmlFile.WriteLine("</HTML>");


}
