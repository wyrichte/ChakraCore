our %OPTIONS = (
    # Lab account details
    ServiceAccountUser  => "REDMOND\\chakraut", # Fully qualified domain name of the service account
    ServiceAccountEmail => 'ChakrAut@microsoft.com', # Email address of the service account (to send from)

    # Email options
    EmailTo             => 'tcare@microsoft.com',
    EmailCC             => ['curtism@microsoft.com', 'hiteshk@microsoft.com'],
    Email               => 1,

    # Enlistment options
    RootSDEnlistment    => "C:\\dev3.sd\\chakra",
    CoreSDEnlistment    => "C:\\dev3.sd\\chakracore",
    VSOCoreURL          => "https://devdiv.visualstudio.com/DefaultCollection/DevDiv/_git/ChakraCore",
    VSOPrivateURL       => "https://devdiv.visualstudio.com/DefaultCollection/DevDiv/_git/Chakra",

    # General run options
    Verbose             => 1,
    DryRun              => 0,
    SNAP                => 0,
    Cleanup             => 0,

    # Misc
    NTDEVAliases        => [qw(yongqu tawoll)], # List of aliases to prefix with NTDEV
);