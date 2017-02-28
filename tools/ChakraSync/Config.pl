our %OPTIONS = (
    # Lab account details
    ServiceAccountUser  => "REDMOND\\chakraut", # Fully qualified domain name of the service account
    ServiceAccountEmail => 'ChakrAut@microsoft.com', # Email address of the service account (to send from)

    # Scheduled task settings
    AutoTaskName        => 'RS3 Git to SD Limited Auto Sync',

    # Email options
    EmailTo             => ['tcare@microsoft.com', 'cc:curtism@microsoft.com', 'cc:hiteshk@microsoft.com', 'cc:doilij@microsoft.com'],
    Email               => 1,

    # Enlistment options
    SDEnlistment        => "C:\\rs3_dev3\\onecoreuap\\inetcore\\jscript",
    RemoteGitURL        => "https://devdiv.visualstudio.com/DefaultCollection/DevDiv/_git/Chakra",
    FullBranch          => 'unreleased/rs3', # Remote working Git branch
    CoreBranch          => 'unreleased/rs3', # Used to override the association of the Full branch with the Core one. E.g. after a branch rename
    GitHubURL           => 'https://chakrabot:<censored>@github.com/Microsoft/ChakraCore.git', # VSO Sync key
    GitHubRepoName      => 'ChakraCore',
    VSOCoreURL          => 'https://devdiv.visualstudio.com/DefaultCollection/DevDiv/_git/ChakraCore',

    # General run options
    Verbose             => 1,
    DryRun              => 0,
    Cleanup             => 0,

    # Misc
    NTDEVAliases        => [qw(yongqu tawoll)], # List of aliases to prefix with NTDEV
);