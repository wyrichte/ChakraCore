-------------------------------------------------------------------------
-- <copyright file="UnitTestFailureTable.sql" company="Microsoft">
--     Copyright (C) Microsoft 2013, All rights reserved.
-- </copyright>
-------------------------------------------------------------------------

-- Table that stores records for unit test failures from SNAP and
-- local user runs.
CREATE TABLE [dbo].[UnitTestFailureTable]
(
    [Id] INT NOT NULL PRIMARY KEY IDENTITY, 
    [UserName] NVARCHAR(256) NOT NULL, 
    [WasRunInSnap] BIT NOT NULL, 
    [StartTime] DATETIME2 NOT NULL, 
    [EndTime] DATETIME2 NOT NULL, 
    [Duration] TIME NOT NULL, 
    [FullCommandLine] NVARCHAR(2048) NOT NULL, 
    [Variant] NVARCHAR(256) NOT NULL, 
    [CommandLineParameters] NVARCHAR(2048) NOT NULL, 
    [UnitTestFileName] NVARCHAR(256) NOT NULL, 
    [HostType] NVARCHAR(256) NOT NULL, 
    [ErrorMessage] NVARCHAR(MAX) NOT NULL, 
    [MachineName] NVARCHAR(256) NOT NULL, 
    [OperatingSystemVersion] NVARCHAR(256) NOT NULL,
    [LogFilePath] NVARCHAR(256) NOT NULL, 
    [SnapJobId] INT NOT NULL, 
    [SnapCheckinId] NVARCHAR(64) NOT NULL, 
    [BuildArchitecture] NVARCHAR(64) NOT NULL, 
    [BuildType] NVARCHAR(64) NOT NULL, 
    [BranchName] NVARCHAR(256) NOT NULL,
    CONSTRAINT UniqueConstraint_UniqueFailureRecord UNIQUE ([StartTime],[FullCommandLine],[SnapJobId],[BranchName],[BuildArchitecture],[BuildType])
)
