
# Telemetry Workflow Generation Tooling

This folder contains the necessary components to generate the Asimov XFlow. The
GenTel script will read a couple of files from both core and full and use them,
through a set of regexes and templates, to generate the actual SCOPE scripts we
use to process the incoming telemetry stream. You can then use XFlowConfig as a
way to upload the generated scripts to Asimov, or open them in SCOPE Studio and
verify their functionality or test a one-off change.

## Permissions
You need to join the following security groups from https://idweb/
  - "OSG Data Full"

## Layout:
output - Where the generated workflows end up
templates - Partial SCOPE files for custom template system
direct - Full SCOPE files that don't need fix-ups; directly copied to output
GenTel.py - Python script that contains the logic needed to generate output


##Installing/Updating XFlow Command-Line Tools

XFlowConfig command-line tools & dependencies can be downloaded & installed 
via the script:

`scripts\getTools.cmd`. 

More information (including latest version updates & how to install via NuGet) 
are available
[here](https://osgwiki.com/wiki/XFlow/DeploymentTool#If_you_wish_to_use_XflowConfig_as_a_Command_Line_Tool).   

### Manual Instructions:

1. Open browser to  [XFlowConfig Nuget Page](https://microsoft.visualstudio.com/OSGS/ft_Xflow/_packaging?feed=Universal.Store&package=36b53826-24f2-464e-80fc-8763578e36ba&version=0ab7aa7c-2f94-43cd-b6c3-af97a6e60c3e&_a=package)
2. Download File via link at   Run `nuget.exe sources Add -Name "Universal.Store" -Source "https://microsoft.pkgs.visualstudio.com/_packaging/Universal.Store/nuget/v3/index.json"`
3. Rename xflowconfig.<version>.nupkg to a xflowconfig.zip
4. Extract file
5. Copy the "Tools" folder from the extraction to `\chakra\tools\Telemetry\XFlowConfig\`

## Useful commands:

Export the current workflow state over your local one, so you can check for any
uncommitted changes:

`scripts\export.cmd`

Import your new workflow set-up over the remote one, making it live on the next
set of jobs to be kicked off:

## Data Egress From Cosmos -> Kusto

  - Add stream to existing WPTChakraKustoEgress job:  https://datagrid/Egress/Egress/Edit?jobId=5bab099b-d140-4d5d-9625-2771249d048e
    - Alternatively, create a new job:  https://datagrid/Egress/ 
      - create a new job an select Kusto Cluster as the storage type.
      - Cluster URL: https://wdgeventstore.kusto.windows.net:443
      - Data Mart: Edge
      - No credential needed here.
  - For cosmos Stream: 
    - fill it out as you would any other egress job using the proper creds based on where it’s stored:  
    - See here for more details: https://osgwiki.com/wiki/WebPlatfrom_DataGrid_Egress_Guidelines#Set_up_the_stream_information 
    - Our table naming convention is WPT_Chakra_<tablename>
