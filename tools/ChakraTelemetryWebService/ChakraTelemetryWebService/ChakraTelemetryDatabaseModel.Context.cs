﻿//------------------------------------------------------------------------------
// <auto-generated>
//    This code was generated from a template.
//
//    Manual changes to this file may cause unexpected behavior in your application.
//    Manual changes to this file will be overwritten if the code is regenerated.
// </auto-generated>
//------------------------------------------------------------------------------

namespace ChakraTelemetryWebService
{
    using System;
    using System.Data.Entity;
    using System.Data.Entity.Infrastructure;
    using System.Data.Objects;
    using System.Data.Objects.DataClasses;
    using System.Linq;
    
    public partial class ChakraTelemetryDatabaseEntities : DbContext
    {
        public ChakraTelemetryDatabaseEntities()
            : base("name=ChakraTelemetryDatabaseEntities")
        {
        }
    
        protected override void OnModelCreating(DbModelBuilder modelBuilder)
        {
            throw new UnintentionalCodeFirstException();
        }
    
    
        public virtual int RecordUnitTestFailure(string userName, Nullable<bool> wasRunInSnap, Nullable<System.DateTime> startTime, Nullable<System.DateTime> endTime, Nullable<System.TimeSpan> duration, string fullCommandLine, string variant, string commandLineParameters, string unitTestFileName, string hostType, string errorMessage, string machineName, string operatingSystemVersion, string logFilePath, Nullable<int> snapJobId, string snapCheckinId, string buildArchitecture, string buildType, string branchName)
        {
            var userNameParameter = userName != null ?
                new ObjectParameter("userName", userName) :
                new ObjectParameter("userName", typeof(string));
    
            var wasRunInSnapParameter = wasRunInSnap.HasValue ?
                new ObjectParameter("wasRunInSnap", wasRunInSnap) :
                new ObjectParameter("wasRunInSnap", typeof(bool));
    
            var startTimeParameter = startTime.HasValue ?
                new ObjectParameter("startTime", startTime) :
                new ObjectParameter("startTime", typeof(System.DateTime));
    
            var endTimeParameter = endTime.HasValue ?
                new ObjectParameter("endTime", endTime) :
                new ObjectParameter("endTime", typeof(System.DateTime));
    
            var durationParameter = duration.HasValue ?
                new ObjectParameter("duration", duration) :
                new ObjectParameter("duration", typeof(System.TimeSpan));
    
            var fullCommandLineParameter = fullCommandLine != null ?
                new ObjectParameter("fullCommandLine", fullCommandLine) :
                new ObjectParameter("fullCommandLine", typeof(string));
    
            var variantParameter = variant != null ?
                new ObjectParameter("variant", variant) :
                new ObjectParameter("variant", typeof(string));
    
            var commandLineParametersParameter = commandLineParameters != null ?
                new ObjectParameter("commandLineParameters", commandLineParameters) :
                new ObjectParameter("commandLineParameters", typeof(string));
    
            var unitTestFileNameParameter = unitTestFileName != null ?
                new ObjectParameter("unitTestFileName", unitTestFileName) :
                new ObjectParameter("unitTestFileName", typeof(string));
    
            var hostTypeParameter = hostType != null ?
                new ObjectParameter("hostType", hostType) :
                new ObjectParameter("hostType", typeof(string));
    
            var errorMessageParameter = errorMessage != null ?
                new ObjectParameter("errorMessage", errorMessage) :
                new ObjectParameter("errorMessage", typeof(string));
    
            var machineNameParameter = machineName != null ?
                new ObjectParameter("machineName", machineName) :
                new ObjectParameter("machineName", typeof(string));
    
            var operatingSystemVersionParameter = operatingSystemVersion != null ?
                new ObjectParameter("operatingSystemVersion", operatingSystemVersion) :
                new ObjectParameter("operatingSystemVersion", typeof(string));
    
            var logFilePathParameter = logFilePath != null ?
                new ObjectParameter("logFilePath", logFilePath) :
                new ObjectParameter("logFilePath", typeof(string));
    
            var snapJobIdParameter = snapJobId.HasValue ?
                new ObjectParameter("snapJobId", snapJobId) :
                new ObjectParameter("snapJobId", typeof(int));
    
            var snapCheckinIdParameter = snapCheckinId != null ?
                new ObjectParameter("snapCheckinId", snapCheckinId) :
                new ObjectParameter("snapCheckinId", typeof(string));
    
            var buildArchitectureParameter = buildArchitecture != null ?
                new ObjectParameter("buildArchitecture", buildArchitecture) :
                new ObjectParameter("buildArchitecture", typeof(string));
    
            var buildTypeParameter = buildType != null ?
                new ObjectParameter("buildType", buildType) :
                new ObjectParameter("buildType", typeof(string));
    
            var branchNameParameter = branchName != null ?
                new ObjectParameter("branchName", branchName) :
                new ObjectParameter("branchName", typeof(string));
    
            return ((IObjectContextAdapter)this).ObjectContext.ExecuteFunction("RecordUnitTestFailure", userNameParameter, wasRunInSnapParameter, startTimeParameter, endTimeParameter, durationParameter, fullCommandLineParameter, variantParameter, commandLineParametersParameter, unitTestFileNameParameter, hostTypeParameter, errorMessageParameter, machineNameParameter, operatingSystemVersionParameter, logFilePathParameter, snapJobIdParameter, snapCheckinIdParameter, buildArchitectureParameter, buildTypeParameter, branchNameParameter);
        }
    }
}
