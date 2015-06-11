using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Data.SqlClient;
using System.IO;
namespace RuntimeDataToSql
{
    class Program
    {
        private static BinaryReader binaryReader;
        private static SqlConnection sqlConnection = null;
        private static SqlCommand sqlCommand = new SqlCommand();
        private static string inputFilename = null;
        private static string dataSource = "localhost\\SQLEXPRESS";
        private static string databaseName = "RuntimeData";
        private static FileStream inputFile = null;
        private static bool resetTable = false;
        private static bool uniqueCheck = false;
        private static uint resumePosition = 0;
        private static void PrintUsage()
        {
            Console.WriteLine("Usage: DynamicProfileDataToSql <options>");
            Console.WriteLine("Options:");
            Console.WriteLine("  -server <name>     SQL server instance (default: localhost\\SQLEXPRESS)");
            Console.WriteLine("  -database <name>   Database name (default: RuntimeData)");
            Console.WriteLine("  -reset             Clear all existing data");
            Console.WriteLine("  -resume <pos>      Resume importing data from input file position <pos>");
            Console.WriteLine("  -unique            Check data consistancy (unique constraints) while importing");
            Console.WriteLine("  -import <filename> Import file name");
        }
        private static bool ParseArgs(string[] args)
        {
            if (args.Length < 1)
            {
                PrintUsage();
                return false;
            }
            for (uint i = 0; i < args.Length; i++)
            {
                string arg = args[i].ToLower();
                if (arg == "-reset")
                {
                    resetTable = true;
                    continue;
                }
                if (arg == "-unique")
                {
                    uniqueCheck = true;
                    continue;
                }
                if (arg == "-resume")
                {
                    i++;
                    if (i > args.Length)
                    {
                        Console.WriteLine("ERROR: missing argument for -resume");
                        return false;
                    }
                    try
                    {
                        resumePosition = Convert.ToUInt32(args[i]);
                    }
                    catch (Exception)
                    {
                        Console.WriteLine("ERROR: invalid argument for -resume {0}", args[i]);
                        return false;
                    }
                    continue;
                }
                if (arg == "-import")
                {
                    i++;
                    if (i > args.Length)
                    {
                        Console.WriteLine("ERROR: missing argument for -import");
                        return false;
                    }
                    if (inputFilename != null)
                    {
                        Console.WriteLine("ERROR: multiple -import not supported");
                        return false;
                    }
                    inputFilename = args[i];
                    continue;
                }
                if (arg == "-server")
                {
                    i++;
                    if (i > args.Length)
                    {
                        Console.WriteLine("ERROR: missing argument for -server");
                        return false;
                    }
                    dataSource = args[i];
                    continue;
                }
                if (arg == "-database")
                {
                    i++;
                    if (i > args.Length)
                    {
                        Console.WriteLine("ERROR: missing argument for -database");
                        return false;
                    }
                    databaseName = args[i];
                    continue;
                }
                Console.WriteLine("ERROR: invalid parameter {0}", args[i]);
                return false;

            }
            
            return true;
        }        
        private static bool OpenSqlConnection()
        {
            sqlConnection = new SqlConnection("Data Source=" + dataSource + ";Initial Catalog=" + databaseName + ";Integrated Security=True");

            try
            {
                sqlConnection.Open();                
                sqlCommand.Connection = sqlConnection;
                InitInsertElemAccessCommand();
                InitInsertCallSiteInfoCommand();
                InitAddFunctionCommand();
                InitSetFunctionInfoCommand();
            }
            catch (Exception e)
            {
                Console.WriteLine("ERROR: Failed to connect to SQL server: " + e.ToString());
                return false;
            }
            return true;
        }
        private static bool OpenInputFile()
        {
            try
            {
                inputFile = File.Open(inputFilename, FileMode.Open, FileAccess.Read, FileShare.Read);                
                binaryReader = new BinaryReader(inputFile, Encoding.UTF8);
            }
            catch (Exception e)
            {
                Console.WriteLine("ERROR: Opening input file '" + inputFilename + "': " + e.ToString());
                return false;
            }
            return true;
        }
        private static DateTime ReadDateTime()
        {
            uint time = binaryReader.ReadUInt32();
            DateTime dt = new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc);
            return dt.AddSeconds(time).ToLocalTime();            
        }
        private static string ReadString()
        {
            uint len = binaryReader.ReadUInt32();
            if (len == 0)
            {
                return "";
            }
#if BUG
            char[] s = binaryReader.ReadChars((int)len);
            return new String(s);
#else
            byte[] utf8String = binaryReader.ReadBytes((int)len);
            return UTF8Encoding.UTF8.GetString(utf8String);       
#endif     
      
        }

        private static uint[] ReadUInt32Array()
        {
            var count = binaryReader.ReadUInt32();
            uint[] arr = new uint[count];
            for (var i = 0; i < count; i++)
            {
                arr[i] = binaryReader.ReadUInt32();
            }
            return arr;
        }
        private static byte[] ReadByteArray()
        {
            var count = binaryReader.ReadUInt32();
            return binaryReader.ReadBytes((int)count);
        }

        private static SqlCommand insertElemAccessCommand = new SqlCommand();
        private static SqlParameter sessionFunctionIdParam = new SqlParameter("@sessionFunctionId", System.Data.SqlDbType.Int);
        private static SqlParameter elemAccessIdParam = new SqlParameter("@elemAccessId", System.Data.SqlDbType.Int);
        private static SqlParameter isArrayParam = new SqlParameter("@isArray", System.Data.SqlDbType.Bit);
        private static SqlParameter isLocalParam = new SqlParameter("@isLocal", System.Data.SqlDbType.Bit);
        private static void InitInsertElemAccessCommand()
        {
            insertElemAccessCommand.CommandText = "INSERT INTO " + SessionFunctionArrayAccessTableName + " Values (@sessionFunctionId,@elemAccessId,@isArray,@isLocal)";
            insertElemAccessCommand.Connection = sqlConnection;            
            insertElemAccessCommand.Parameters.Add(sessionFunctionIdParam);
            insertElemAccessCommand.Parameters.Add(elemAccessIdParam);
            insertElemAccessCommand.Parameters.Add(isArrayParam);            
            insertElemAccessCommand.Parameters.Add(isLocalParam);
        }
        private static void InsertElemAccess(byte[] elemAccess, int sessionFunctionId, bool isLocal)
        {
            if (elemAccess.Length != 0)
            {                 
                insertElemAccessCommand.Transaction = sqlCommand.Transaction;
                
                for (var i = 0; i < elemAccess.Length; i++)
                {                   
                    sessionFunctionIdParam.Value = sessionFunctionId;
                    elemAccessIdParam.Value = i;
                    isArrayParam.Value = (elemAccess[i] == 1 ? (object)true : elemAccess[i] == Byte.MaxValue ? (object)false : DBNull.Value);
                    isLocalParam.Value = isLocal;
                    insertElemAccessCommand.ExecuteNonQuery();                    
                }                
            }
        }

        private static int AddUrlToTable(string table, string url)
        {
            sqlCommand.CommandText = "DECLARE @ItemID int;" +
                    "SET @ItemID =( SELECT ID FROM " + table + " WHERE URL=@url);" +
                    "IF (@ItemID IS NULL) BEGIN " +
                        "INSERT INTO " + table + " Values (@url);" +
                        "SET @ItemID = @@IDENTITY;" +
                    "END;" +
                    "SELECT @ItemID";
            sqlCommand.Parameters.AddWithValue("@url", url);
            var result = sqlCommand.ExecuteScalar();
            sqlCommand.Parameters.Clear();
            return Convert.ToInt32(result);
        }

        // AddFunction
        private static SqlCommand addFunctionCommand = new SqlCommand();
        private static SqlParameter addFunctionSessionSourceIDParam = new SqlParameter("@sessionSourceID", System.Data.SqlDbType.Int);
        private static SqlParameter addFunctionFunctionIDParam = new SqlParameter("@functionID", System.Data.SqlDbType.Int);     
        private static void InitAddFunctionCommand()
        {            
            addFunctionCommand.Connection = sqlConnection;
            addFunctionCommand.CommandText = "INSERT INTO " + SessionFunctionTableName + " Values (@sessionSourceID,@functionID,NULL,NULL);" +
                "SELECT @@IDENTITY";
            addFunctionCommand.Parameters.Add(addFunctionSessionSourceIDParam);
            addFunctionCommand.Parameters.Add(addFunctionFunctionIDParam);
        }
        private static int AddFunction(int sessionSourceID, uint functionID)
        {
            addFunctionCommand.Transaction = sqlCommand.Transaction;
            addFunctionSessionSourceIDParam.Value = sessionSourceID;
            addFunctionFunctionIDParam.Value = (int)functionID;
            var result = addFunctionCommand.ExecuteScalar();            
            return Convert.ToInt32(result);
        }

        // SetFunctionInfo
        private static SqlCommand setFunctionInfoCommand = new SqlCommand();
        private static SqlParameter functionNameParam = new SqlParameter("@functionName", System.Data.SqlDbType.NVarChar);
        private static SqlParameter interpretedCountParam = new SqlParameter("@interpretedCount", System.Data.SqlDbType.Int);
        private static SqlParameter setFunctionInfoSessionFunctionIDParam = new SqlParameter("@sessionFunctionID", System.Data.SqlDbType.Int);
        private static void InitSetFunctionInfoCommand()
        {
            setFunctionInfoCommand.Connection = sqlConnection;
            setFunctionInfoCommand.CommandText= "UPDATE " + SessionFunctionTableName + " SET Name=@functionName,InterpretedCount=@interpretedCount  FROM " + SessionFunctionTableName +
                       " WHERE ID=@sessionFunctionID AND Name IS NULL AND InterpretedCount IS NULL";
            setFunctionInfoCommand.Parameters.Add(functionNameParam);
            setFunctionInfoCommand.Parameters.Add(interpretedCountParam);
            setFunctionInfoCommand.Parameters.Add(setFunctionInfoSessionFunctionIDParam);
        }
        private static bool SetFunctionInfo(int sessionFunctionID, string functionName, uint interpretedCount)
        {
            setFunctionInfoCommand.Transaction = sqlCommand.Transaction;
            functionNameParam.Value = functionName;
            interpretedCountParam.Value = (int)interpretedCount;
            setFunctionInfoSessionFunctionIDParam.Value = sessionFunctionID;
            return setFunctionInfoCommand.ExecuteNonQuery() == 1;            
        }

        // Insert call site
        private static SqlCommand insertCallSiteInfoCommand = new SqlCommand();
        private static SqlParameter callerSessionFunctionIdParam = new SqlParameter("@sessionFunctionID", System.Data.SqlDbType.Int);
        private static SqlParameter callSiteIDParam = new SqlParameter("@callSiteID", System.Data.SqlDbType.Int);
        private static SqlParameter calleeSessionFunctionIDParam = new SqlParameter("@calleeSessionFunctionID", System.Data.SqlDbType.Int);
        private static SqlParameter nonBodyParam = new SqlParameter("@nonBody", System.Data.SqlDbType.TinyInt);
        private static void InitInsertCallSiteInfoCommand()
        {
            insertCallSiteInfoCommand.CommandText = "INSERT INTO " + SessionFunctionCallSiteTableName + " Values (@sessionFunctionID,@callSiteID,@calleeSessionFunctionID,@nonBody)";
            insertCallSiteInfoCommand.Connection = sqlConnection;
            insertCallSiteInfoCommand.Parameters.Add(callerSessionFunctionIdParam);
            insertCallSiteInfoCommand.Parameters.Add(callSiteIDParam);            
            insertCallSiteInfoCommand.Parameters.Add(calleeSessionFunctionIDParam);            
            insertCallSiteInfoCommand.Parameters.Add(nonBodyParam);
        }

        private static long currentRecordPosition = 0;
        private static bool DoImport()
        {
            inputFile.Position = resumePosition;
            var timer = new System.Diagnostics.Stopwatch();
            timer.Start();
            long interval = inputFile.Length / 10;
            long next = 1;
            uint sessionCount = 0;
            uint skippedSessionCount = 0;
            while (inputFile.Position < inputFile.Length)
            {
                currentRecordPosition = inputFile.Position;
                if (next * interval < inputFile.Position)
                {
                    Console.WriteLine("{0}0% done", next);
                    next++;
                }
                var sqlTransaction = sqlConnection.BeginTransaction();
                try
                {
                    bool skipRecord = false;
                    sqlCommand.Transaction = sqlTransaction;

                    var scriptContextToken = binaryReader.ReadInt32();
                    var dt = ReadDateTime();
                    var url = ReadString();
                    var scriptContextID = AddUrlToTable(ScriptContextTableName, url);
                
                    sqlCommand.CommandText = "DECLARE @SessionID int;" +
                        "SET @SessionID = (SELECT ID FROM " + SessionTableName + " WHERE Time=@time AND ScriptContextID=@scriptContextID AND Token=@scriptContextToken);" +
                        "IF @SessionID IS NOT NULL " +
                            "SELECT 0;" +
                        "ELSE BEGIN " +
                            "INSERT INTO " + SessionTableName + " Values (@time,@scriptContextID,@scriptContextToken);" +
                            "SELECT @@IDENTITY;" +
                        "END";

                    sqlCommand.Parameters.AddWithValue("@time", dt);
                    sqlCommand.Parameters.AddWithValue("@scriptContextID", scriptContextID);                    
                    sqlCommand.Parameters.AddWithValue("@scriptContextToken", scriptContextToken);
                    var result = sqlCommand.ExecuteScalar();
                    sqlCommand.Parameters.Clear();
                    var sessionID = Convert.ToInt32(result);
                    if (sessionID == 0)
                    {
                        skipRecord = true;
                    }

                    var sourceMapCount = binaryReader.ReadUInt32();
                    var sourceMap = new Dictionary<uint, int>();
                    for (var i = 0; i < sourceMapCount; i++)
                    {
                        var hostSourceContext = binaryReader.ReadUInt32();
                        var functionCount = binaryReader.ReadUInt32();                    
                        var sourceUrl = ReadString();

                        if (!skipRecord)
                        {
                            if (sourceMap.ContainsKey(hostSourceContext))
                            {
                                Console.WriteLine("ERROR: File corrupted at {0}({1}): multiple host source context", currentRecordPosition, inputFile.Position);
                                return false;
                            }

                            var sourceContextID = AddUrlToTable(SourceContextTableName, sourceUrl);
                            sqlCommand.CommandText =
                                "INSERT INTO " + SessionSourceTableName + " Values (@sessionID,@sourceContextID,@functionCount);" +
                                "SELECT @@IDENTITY";
                            sqlCommand.Parameters.AddWithValue("@sessionID", sessionID);
                            sqlCommand.Parameters.AddWithValue("@sourceContextID", sourceContextID);
                            sqlCommand.Parameters.AddWithValue("@functionCount", (int)functionCount);
                            result = sqlCommand.ExecuteScalar();
                            sqlCommand.Parameters.Clear();
                            var sessionSourceID = Convert.ToInt32(result);
                            sourceMap.Add(hostSourceContext, sessionSourceID);
                        }
                    }

                    var sential = binaryReader.ReadByte();

                    var functionMap = new Dictionary<KeyValuePair<int, uint>, int>();                    
                    while (sential != 0)
                    {
                        if (sential > 1)
                        {
                            Console.WriteLine("ERROR: File corrupted at {0}({1}): invalid sential value {2}", currentRecordPosition, inputFile.Position, sential);                        
                            return false;
                        }
                        var functionHostSourceContext = binaryReader.ReadUInt32();                    
                        var functionId = binaryReader.ReadUInt32();
                        var functionName = ReadString();
                        var interpretedCount = binaryReader.ReadUInt32();
                        int sessionFunctionID = 0;
                        if (!skipRecord)
                        {
                            int sessionSourceID;
                            if (!sourceMap.TryGetValue(functionHostSourceContext, out sessionSourceID))
                            {
                                Console.WriteLine("ERROR: File corrupted at {0}({1}): invalid function host source context {2}", currentRecordPosition, inputFile.Position, functionHostSourceContext);
                                return false;
                            }                            
                            var functionIdPair = new KeyValuePair<int, uint>(sessionSourceID, functionId);
                            if (!functionMap.TryGetValue(functionIdPair, out sessionFunctionID))
                            {
                                sessionFunctionID = AddFunction(sessionSourceID, functionId);
                                functionMap.Add(functionIdPair, sessionFunctionID);
                            }
                                
                            if (!SetFunctionInfo(sessionFunctionID, functionName, interpretedCount))
                            {
                                Console.WriteLine("ERROR: File corrupted at {0}({1}): multiple function info", currentRecordPosition, inputFile.Position);
                                return false;
                            }
                        }  
                        var loopBodyEntryCount = ReadUInt32Array();
                        
                        
                        if (!skipRecord)
                        {
                            for (var i = 0; i < loopBodyEntryCount.Length; i++)
                            {
                                // TODO : sqlCommand.CommandText = "INSERT INTO " + SeloopBodyEntryCount[i] +;
                            }
                        }
                        var localElemAccess = ReadByteArray();
                        if (!skipRecord)
                        {
                            InsertElemAccess(localElemAccess, sessionFunctionID, true);
                        }

                        var nonLocalElemAccess = ReadByteArray();
                        if (!skipRecord)
                        {
                            InsertElemAccess(nonLocalElemAccess, sessionFunctionID, false);
                        }
                        
                        // Call site info
                        insertCallSiteInfoCommand.Transaction = sqlTransaction;
                        callerSessionFunctionIdParam.Value = sessionFunctionID;
                        var callSiteCount = binaryReader.ReadUInt32();
                        for (var i = 0; i < callSiteCount; i++)
                        {
                            callSiteIDParam.Value = (int)i;
                            var isFunctionBody = binaryReader.ReadBoolean();
                            if (isFunctionBody)
                            {
                                var calleeFunctionHostContext = binaryReader.ReadUInt32();
                                var calleeFunctionID = binaryReader.ReadUInt32();
                                if (!skipRecord)
                                {
                                    int calleeSessionSourceID;
                                    if (!sourceMap.TryGetValue(calleeFunctionHostContext, out calleeSessionSourceID))
                                    {
                                        Console.WriteLine("ERROR: File corrupted at {0}({1}): invalid function host source context {2}", currentRecordPosition, inputFile.Position, calleeFunctionHostContext);
                                        return false;
                                    }
                                    int calleeSessionFunctionID;
                                    var calleeFunctionIdPair = new KeyValuePair<int, uint>(calleeSessionSourceID, calleeFunctionID);
                                    if (!functionMap.TryGetValue(calleeFunctionIdPair, out calleeSessionFunctionID))
                                    {
                                        calleeSessionFunctionID = AddFunction(calleeSessionSourceID, calleeFunctionID);
                                        functionMap.Add(calleeFunctionIdPair, calleeSessionFunctionID);
                                    }
                                           
                                    calleeSessionFunctionIDParam.Value = calleeSessionFunctionID;
                                    nonBodyParam.Value = DBNull.Value;
                                    insertCallSiteInfoCommand.ExecuteNonQuery();                            
                                }
                            }
                            else
                            {
                                var nonBodyCall = binaryReader.ReadByte();
                                if (!skipRecord)
                                { 
                                    calleeSessionFunctionIDParam.Value = DBNull.Value;
                                    nonBodyParam.Value = nonBodyCall;
                                    insertCallSiteInfoCommand.ExecuteNonQuery();
                                }                                
                            }                            
                        }

                        sential = binaryReader.ReadByte();
                    }

                    sessionCount++;
                    if (skipRecord)
                    {
                        skippedSessionCount++;
                    }
                    sqlTransaction.Commit();
                    
                }
                finally
                {
                    sqlTransaction.Dispose();
                    sqlCommand.Transaction = null;
                }
            }
            timer.Stop();
            Console.WriteLine("Elapsed Time   : {0} ms", timer.ElapsedMilliseconds);
            Console.WriteLine("Total Session  : {0}", sessionCount);
            if (skippedSessionCount != 0)
            {
                Console.WriteLine("Skipped Session: {0}", skippedSessionCount);
            }
            return true;
        }


        private static readonly string ScriptContextTableName = "ScriptContexts";
        private static readonly string SourceContextTableName = "SourceContexts";
        private static readonly string SessionTableName = "Sessions";        
        private static readonly string SessionSourceTableName = "SessionSources";        
        private static readonly string SessionFunctionTableName = "SessionFunctions";
        private static readonly string SessionFunctionCallSiteTableName = "SessionFunctionCallSites";
        private static readonly string SessionFunctionArrayAccessTableName = "SessionFunctionArrayAccesses";


        private static readonly string UniqueSessionView = "UniqueSessions";
        private static readonly string UniqueSessionSourcesView = "UniqueSessionSources";
        private static readonly string UniqueSessionSourceDetailView = "UniqueSessionSourcesDetail";
        private static readonly string SourceReferenceCountView = "SourceReferenceCount";
        private static readonly string SourceReferenceCountDetailView = "SourceReferenceCountDetail";
        private static readonly string MultiRefSourcesView = "MultiRefSourcesView";
        private static readonly string ArrayElemAccessView = "ArrayElemAccesses";
        private static readonly string ArrayElemAccessFromMultiRefSourcesView = "ArrayElemAccessesFromMultiRefSources";
        private static void DropTable(string tableName)
        {                        
            sqlCommand.CommandText = "IF object_id('" + tableName + "', 'U') is not null DROP TABLE " + tableName;                     
            sqlCommand.ExecuteNonQuery();
        }
        private static void CreateTable(string tableName, string columnSpec)
        {            
            sqlCommand.CommandText = "CREATE TABLE dbo." + tableName + " (" + columnSpec + ")";
            sqlCommand.ExecuteNonQuery();
        }
        private static void DropView(string viewName)
        {
            sqlCommand.CommandText = "IF object_id('" + viewName + "', 'V') is not null DROP VIEW " + viewName;
            sqlCommand.ExecuteNonQuery();
        }
        private static void CreateView(string viewName, string viewQuery)
        {
            sqlCommand.CommandText = "CREATE VIEW dbo." + viewName + " WITH SCHEMABINDING AS " + viewQuery;
            sqlCommand.ExecuteNonQuery();
        }
        private static bool CreateTables()
        {
            try
            {
                var sqlTransaction = sqlConnection.BeginTransaction();
                sqlCommand.Transaction = sqlTransaction;
                DropView(ArrayElemAccessFromMultiRefSourcesView);
                DropView(ArrayElemAccessView);
                DropView(MultiRefSourcesView);
                DropView(SourceReferenceCountDetailView);
                DropView(SourceReferenceCountView);
                DropView(UniqueSessionSourceDetailView);
                DropView(UniqueSessionSourcesView);
                DropView(UniqueSessionView);
                DropTable(SessionFunctionArrayAccessTableName);
                DropTable(SessionFunctionCallSiteTableName);
                DropTable(SessionFunctionTableName);
                DropTable(SessionSourceTableName);
                DropTable(SessionTableName);
                DropTable(ScriptContextTableName);
                DropTable(SourceContextTableName);
                sqlTransaction.Commit();
            }
            catch (Exception e)
            {
                Console.WriteLine("ERROR: Unable to drop existing tables: " + e.ToString());
                return false;
            }

            try
            {
                var sqlTransaction = sqlConnection.BeginTransaction();
                sqlCommand.Transaction = sqlTransaction;
                CreateTable(ScriptContextTableName, "ID INT IDENTITY PRIMARY KEY, URL nvarchar(MAX) NOT NULL");
                CreateTable(SourceContextTableName, "ID INT IDENTITY PRIMARY KEY, URL nvarchar(MAX) NOT NULL");
                CreateTable(SessionTableName, "ID INT IDENTITY PRIMARY KEY, Time DATETIME, ScriptContextID INT NOT NULL FOREIGN KEY REFERENCES dbo." + ScriptContextTableName + " (ID), Token BIGINT"
                    + (uniqueCheck ? " CONSTRAINT UQ_" + SessionTableName + " UNIQUE " + SessionTableUniqueConstraint : "")
                    );
                CreateTable(SessionSourceTableName, "ID INT IDENTITY PRIMARY KEY, SessionID INT NOT NULL FOREIGN KEY REFERENCES dbo." + SessionTableName + " (ID), "
                    + "SourceID INT NOT NULL FOREIGN KEY REFERENCES dbo." + SourceContextTableName + " (ID), FunctionCount INT NOT NULL" 
                    //+ (uniqueCheck ? " CONSTRAINT UQ_" + SessionSourceTableName + " UNIQUE " + SessionSourceTableUniqueConstraint : "")
                    );
                CreateTable(SessionFunctionTableName, "ID INT IDENTITY PRIMARY KEY, SessionSourceID INT NOT NULL FOREIGN KEY REFERENCES dbo." + SessionSourceTableName + " (ID), "
                    + "FunctionID INT NOT NULL, Name nvarchar(MAX) NULL, InterpretedCount INT NULL"
                    + (uniqueCheck ? " CONSTRAINT UQ_" + SessionFunctionTableName + " UNIQUE " + SessionFunctionTableUniqueConstraint : "")
                    );
                CreateTable(SessionFunctionCallSiteTableName, "SessionFunctionID INT NOT NULL FOREIGN KEY REFERENCES dbo." + SessionFunctionTableName + " (ID), "
                    + "CallSiteID INT NOT NULL, CalleeSessionFunctionID INT NULL FOREIGN KEY REFERENCES dbo." + SessionFunctionTableName + " (ID), Other TINYINT NULL"
                    + (uniqueCheck ? " CONSTRAINT UQ_" + SessionFunctionCallSiteTableName + " UNIQUE " + SessionFunctionCallSiteTableUniqueConstraint : "")
                    );
                CreateTable(SessionFunctionArrayAccessTableName, "SessionFunctionID INT NOT NULL FOREIGN KEY REFERENCES dbo." + SessionFunctionTableName + " (ID), "
                    + "ElemAccessID INT NOT NULL, IsArrayAccess BIT NULL, IsLocal BIT NOT NULL"
                    + (uniqueCheck ? " CONSTRAINT UQ_" + SessionFunctionArrayAccessTableName + " UNIQUE " + SessionFunctionArrayAccessTableUniqueConstraint : "")
                    );

                // Views to help analysis
                CreateView(UniqueSessionView, String.Format(
                    "SELECT MAX(Sessions.ID) As SessionID, URL FROM dbo.{0} JOIN dbo.{1} ON {0}.ID = {1}.ScriptContextID AND URL != '' GROUP BY URL ", 
                    ScriptContextTableName, SessionTableName));
                CreateView(UniqueSessionSourcesView, String.Format(
                    "SELECT ID, SessionID, SourceID, FunctionCount FROM dbo.{0} WHERE SessionID IN (SELECT SessionID From dbo.{1})", SessionSourceTableName, UniqueSessionView));

                CreateView(UniqueSessionSourceDetailView, String.Format(
                    "SELECT {0}.URL As SessionURL, {1}.URL As SourceURL, FunctionCount, COUNT(SessionFunctions.ID) As CalledFunctionCount, MAX(InterpretedCount) As MaxInterpretedCount " +
                    "FROM dbo.{2} " +
                        "JOIN dbo.{3} ON {2}.SessionID = {3}.ID " +
                        "JOIN dbo.{0} ON {0}.ID = {3}.ScriptContextID " +
                        "JOIN dbo.{1} ON {2}.SourceID = {1}.ID " +
                        "LEFT OUTER JOIN dbo.{4} ON {2}.ID = {4}.SessionSourceID " +
			                    "AND InterpretedCount IS NOT NULL " +
	                    "GROUP BY {0}.URL, {1}.URL, FunctionCount", 
                        ScriptContextTableName, SourceContextTableName, UniqueSessionSourcesView, SessionTableName, SessionFunctionTableName                       
                    ));
                                    
                CreateView(SourceReferenceCountView, String.Format(
                    "SELECT SourceID, FunctionCount, COUNT(*) As ReferenceCount " +
                    "FROM dbo.{0} GROUP BY SourceID, FunctionCount", UniqueSessionSourcesView));

                CreateView(SourceReferenceCountDetailView, String.Format(
                    "SELECT URL, FunctionCount, ReferenceCount " +
                    "FROM dbo.{0} JOIN dbo.{1} ON {1}.ID = {0}.SourceID",
                    SourceReferenceCountView, SourceContextTableName
                    ));
                CreateView(MultiRefSourcesView, String.Format(
                    "SELECT SourceID, FunctionCount, ReferenceCount FROM dbo.{0} " +
                    "WHERE SourceID != (SELECT ID FROM dbo.{1} WHERE URL = '') " +
                    "AND ReferenceCount >=2",
                    SourceReferenceCountView, SourceContextTableName
                    ));
                CreateView(ArrayElemAccessView, String.Format(
                    "SELECT SourceID, FunctionCount, FunctionID, ElemAccessID, IsLocal, COUNT(DISTINCT IsArrayAccess) As ArrayAccessKindCount " +
                    "FROM dbo.{2} " +
                    "INNER JOIN dbo.{1} ON {1}.ID = SessionFunctionID " +
                    "INNER JOIN dbo.{0} ON {0}.ID = SessionSourceID " +
	                "GROUP BY SourceID, FunctionCount, FunctionID, ElemAccessID, IsLocal",
                    SessionSourceTableName, SessionFunctionTableName, SessionFunctionArrayAccessTableName));


                CreateView(ArrayElemAccessFromMultiRefSourcesView, String.Format(
                    "SELECT SourceID, FunctionCount, FunctionID, ElemAccessID, IsLocal, ArrayAccessKindCount " +
                    "FROM dbo.{0} WHERE SourceID IN (SELECT SourceID FROM dbo.{1})",
                    ArrayElemAccessView, MultiRefSourcesView));
                sqlTransaction.Commit();
            }
            catch (Exception e)
            {
                Console.WriteLine("ERROR: Unable to create tables: " + e.ToString());
                return false;
            }
            return true;                       
        }
        private static string SessionTableUniqueConstraint = "(Time,ScriptContextID,Token)";
        // Some site reference the same JS file twice
        //private static string SessionSourceTableUniqueConstraint = "(SessionID, SourceID)";
        private static string SessionFunctionTableUniqueConstraint = "(SessionSourceID, FunctionID)";
        private static string SessionFunctionCallSiteTableUniqueConstraint = "(SessionFunctionID, CallSiteID)";
        private static string SessionFunctionArrayAccessTableUniqueConstraint = "(SessionFunctionID, ElemAccessID, IsLocal)";
        private static void RemoveUniqueCheck(string table)
        {
            sqlCommand.CommandText = "IF object_id('UQ_" + table + "', 'UQ') is not null ALTER TABLE " + table + " DROP CONSTRAINT UQ_" + table;
            sqlCommand.ExecuteNonQuery();
        }
        private static void RemoveUniqueCheck()
        {
            var sqlTransaction = sqlConnection.BeginTransaction();
            sqlCommand.Transaction = sqlTransaction;
            RemoveUniqueCheck(SessionTableName);
            RemoveUniqueCheck(SessionFunctionTableName);
            RemoveUniqueCheck(SessionFunctionCallSiteTableName);
            RemoveUniqueCheck(SessionFunctionArrayAccessTableName);          
            sqlTransaction.Commit();
        }
        private static void AddUniqueCheck(string table, string constraint)
        {
            sqlCommand.CommandText = "IF object_id('UQ_" + table + "', 'UQ') is null ALTER TABLE " + table + " ADD CONSTRAINT UQ_" + table + " UNIQUE " + constraint;
            sqlCommand.ExecuteNonQuery();            
        }
        private static void AddUniqueCheck()
        {
            var sqlTransaction = sqlConnection.BeginTransaction();
            sqlCommand.Transaction = sqlTransaction;
            AddUniqueCheck(SessionTableName, SessionTableUniqueConstraint);
            // AddUniqueCheck(SessionSourceTableName, SessionSourceTableUniqueConstraint);
            AddUniqueCheck(SessionFunctionTableName, SessionFunctionTableUniqueConstraint);
            AddUniqueCheck(SessionFunctionCallSiteTableName, SessionFunctionCallSiteTableUniqueConstraint);
            AddUniqueCheck(SessionFunctionArrayAccessTableName, SessionFunctionArrayAccessTableUniqueConstraint);
            sqlTransaction.Commit();
        }
        static bool Setup()
        {
            try
            {
                if (resetTable)
                {
                    if (!CreateTables())
                    {
                        return false;
                    }
                }
                else
                {
                    if (!uniqueCheck)
                    {
                        RemoveUniqueCheck();
                    }
                    else
                    {
                        AddUniqueCheck();
                    }
                }

            }
            catch (Exception e3)
            {
                Console.WriteLine("ERROR: Unahandled exception modifying table: {0}", e3.ToString());
                return false;
            }
            return true;
        }
        static bool Import()
        {
            if (inputFilename != null)
            {
                if (!OpenInputFile())
                {
                    return false;
                }

                try
                {
                    if (!DoImport())
                    {
                        return false;
                    }

                    if (!uniqueCheck)
                    {
                        // Add back the unique check
                        AddUniqueCheck();
                    }
                }
                catch (Exception e2)
                {
                    Console.WriteLine("ERROR: Unhandled exception importing data at {0}({1}: {2}", currentRecordPosition, inputFile.Position, e2.ToString());
                    return false;
                }
            }
            return true;
        }
        static void Main(string[] args)
        {
            if (!ParseArgs(args))
            {
                return;
            }

            if (inputFilename == null) 
            {
                if (!resetTable)
                {
                    // Nothing to do
                    PrintUsage();
                    return;
                }
                uniqueCheck = true;                                  
            }

            if (!OpenSqlConnection())
            {
                return;
            }

            try
            {
                if (!Setup())
                {
                    return;
                }

                if (!Import())
                {
                    return;
                }
            }
            finally
            {
                if (sqlConnection != null)
                {
                    try
                    {
                        sqlConnection.Close();
                    }
                    catch (Exception e)
                    {
                        Console.WriteLine("ERROR: Failed to close SQL server connection: {0}", e.ToString());
                    }
                }
                if (inputFile != null)
                {
                    try
                    {
                        inputFile.Close();
                    }
                    catch (Exception e)
                    {
                        Console.WriteLine("ERROR: Faile to close input file: {0}", e.ToString());
                    }
                }
            }
        }
    }
}
