-------------------------------------------------------------------------
-- <copyright file="RecordUnitTestFailure.sql" company="Microsoft">
--     Copyright (C) Microsoft 2013, All rights reserved.
-- </copyright>
-------------------------------------------------------------------------

-- Stored procedure that inserts a new unit test failure record
-- into the UnitTestFailureTable, if the entry doesn't already exist.
-- Note that each string parameter is set to max to avoid truncation
-- when passing in.  We do size checks to unsure that it will fit.
CREATE PROCEDURE [dbo].[RecordUnitTestFailure]
    @userName nvarchar(max),
    @wasRunInSnap bit,
    @startTime datetime2(7),
    @endTime datetime2(7),
    @duration time,
    @fullCommandLine nvarchar(max),
    @variant nvarchar(max),
    @commandLineParameters nvarchar(max),
    @unitTestFileName nvarchar(max),
    @hostType nvarchar(max),
    @errorMessage nvarchar(max),
    @machineName nvarchar(max),
    @operatingSystemVersion nvarchar(max),
    @logFilePath nvarchar(max),
    @snapJobId int,
    @snapCheckinId nvarchar(max),
    @buildArchitecture nvarchar(max),
    @buildType nvarchar(max),
    @branchName nvarchar(max)
AS
    -- Size checks for nvarchar(max) parameters.  Need to use DATALENGTH because
    -- LEN will truncate whitespace at the end.  Lengths are multiplied by 2
    -- to account for 2 bytes per character.
    IF DATALENGTH(@userName) > 256 * 2
    BEGIN
       RAISERROR ('Argument @userName is too long to be stored in the table.', 16, 8)
       RETURN (-1)
    END

    IF DATALENGTH(@fullCommandLine) > 2048 * 2
    BEGIN
       RAISERROR ('Argument @fullCommandLine is too long to be stored in the table.', 16, 8)
       RETURN (-1)
    END

    IF DATALENGTH(@variant) > 256 * 2
    BEGIN
       RAISERROR ('Argument @variant is too long to be stored in the table.', 16, 8)
       RETURN (-1)
    END

    IF DATALENGTH(@commandLineParameters) > 2048 * 2
    BEGIN
       RAISERROR ('Argument @commandLineParameters is too long to be stored in the table.', 16, 8)
       RETURN (-1)
    END

    IF DATALENGTH(@unitTestFileName) > 256 * 2
    BEGIN
       RAISERROR ('Argument @unitTestFileName is too long to be stored in the table.', 16, 8)
       RETURN (-1)
    END

    IF DATALENGTH(@hostType) > 256 * 2
    BEGIN
       RAISERROR ('Argument @hostType is too long to be stored in the table.', 16, 8)
       RETURN (-1)
    END

    IF DATALENGTH(@machineName) > 256 * 2
    BEGIN
       RAISERROR ('Argument @machineName is too long to be stored in the table.', 16, 8)
       RETURN (-1)
    END

    IF DATALENGTH(@operatingSystemVersion) > 256 * 2
    BEGIN
       RAISERROR ('Argument @operatingSystemVersion is too long to be stored in the table.', 16, 8)
       RETURN (-1)
    END

    IF DATALENGTH(@logFilePath) > 256 * 2
    BEGIN
       RAISERROR ('Argument @logFilePath is too long to be stored in the table.', 16, 8)
       RETURN (-1)
    END

    IF DATALENGTH(@snapCheckinId) > 64 * 2
    BEGIN
       RAISERROR ('Argument @snapCheckinId is too long to be stored in the table.', 16, 8)
       RETURN (-1)
    END

    IF DATALENGTH(@buildArchitecture) > 64 * 2
    BEGIN
       RAISERROR ('Argument @buildArchitecture is too long to be stored in the table.', 16, 8)
       RETURN (-1)
    END

    IF DATALENGTH(@buildType) > 64 * 2
    BEGIN
       RAISERROR ('Argument @buildType is too long to be stored in the table.', 16, 8)
       RETURN (-1)
    END

    IF DATALENGTH(@branchName) > 256 * 2
    BEGIN
       RAISERROR ('Argument @branchName is too long to be stored in the table.', 16, 8)
       RETURN (-1)
    END

    -- Ensure that the entry has not already been added. Note that we aren't
    -- doing the check + insert as an atomic operation.  This is ok because
    -- we'll only run into problems if more than one tool is using the service
    -- at a time which shouldn't be the case.  Even in that case, the unique contraint
    -- on the table will fail the insert and a message will be returned to the client
    -- indicating such.
    DECLARE @existingEntryCount bigint
    SELECT
        @existingEntryCount = COUNT(*)
    FROM
        [dbo].[UnitTestFailureTable] WITH (NOLOCK)
    WHERE
        [StartTime] = @startTime
        AND
        [FullCommandLine] = @fullCommandLine
        AND
        [SnapJobId] = @snapJobId
        AND
        [BranchName] = @branchName
        AND
        [BuildArchitecture] = @buildArchitecture
        AND
        [BuildType] = @buildType

    IF (@existingEntryCount = 0)
    BEGIN
        INSERT INTO
            [dbo].[UnitTestFailureTable]
        VALUES (
            @userName,
            @wasRunInSnap,
            @startTime,
            @endTime,
            @duration,
            @fullCommandLine,
            @variant,
            @commandLineParameters,
            @unitTestFileName,
            @hostType,
            @errorMessage,
            @machineName,
            @operatingSystemVersion,
            @logFilePath,
            @snapJobId,
            @snapCheckinId,
            @buildArchitecture,
            @buildType,
            @branchName)
    END
RETURN 0
