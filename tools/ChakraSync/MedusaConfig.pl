# This file contains the list of remotes and branches we want to mirror.
# NB: This should only be used for high traffic branches. Mirroring is not an excuse to forget about managing your GitHub branch!

our $PROJECT = 'ChakraCore';

our $PROJECT_OWNERS = ['chakrahot@microsoft.com'];

our $EMAIL_AUTH = {
    Host     => 'smtphost.redmond.corp.microsoft.com',
    From     => 'chakratt@microsoft.com',
    Domain   => 'REDMOND',
    Username => 'chakratt',
    Password => '<censored>'
};

# Interval between reminder, specified in number of attempts. Keep in mind the time between runs of the script when deciding on this number.
our $REMINDER_INTERVAL = '1';

# List of remotes. These must be implicitly able to authenticate, either using a URL embedded username/password or NTLM auth.
our $REMOTES = [
    { Name => 'vso',    URL => 'https://devdiv.visualstudio.com/DefaultCollection/DevDiv/_git/ChakraCore' },
    { Name => 'github', URL => 'https://chakrabot:<censored>@github.com/Microsoft/ChakraCore.git' }
];

# List of branch mappings and owners. The remote must exist in the hash above. CASE SENSITIVE!
our $BRANCH_DATA =  [
    # Core branches used by all of the team. These typically will email chakrahot
    { FromRemote => 'vso', ToRemote => 'github', FromBranch => 'master',      ToBranch => 'master',      Owners => ['chakrahot@microsoft.com'] },
    { FromRemote => 'vso', ToRemote => 'github', FromBranch => 'release/1.1', ToBranch => 'release/1.1', Owners => ['chakrahot@microsoft.com'] },
    { FromRemote => 'vso', ToRemote => 'github', FromBranch => 'release/1.2', ToBranch => 'release/1.2', Owners => ['chakrahot@microsoft.com'] },
    { FromRemote => 'vso', ToRemote => 'github', FromBranch => 'release/1.3', ToBranch => 'release/1.3', Owners => ['chakrahot@microsoft.com'] },

    # Common branches that desire mirroring
    { FromRemote => 'vso', ToRemote => 'github', FromBranch => 'linux',       ToBranch => 'linux',       Owners => ['hiteshk@microsoft.com'] },
    { FromRemote => 'vso', ToRemote => 'github', FromBranch => 'WebAssembly', ToBranch => 'WebAssembly', Owners => ['michhol@microsoft.com'] }
];