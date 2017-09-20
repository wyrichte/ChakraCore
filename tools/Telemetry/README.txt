
Telemetry Workflow Generation Tooling

This folder contains the necessary components to generate the Asimov XFlow. The
GenTel script will read a couple of files from both core and full and use them,
through a set of regexes and templates, to generate the actual SCOPE scripts we
use to process the incoming telemetry stream. You can then use XFlowConfig as a
way to upload the generated scripts to Asimov, or open them in SCOPE Studio and
verify their functionality or test a one-off change.


Layout:
output - Where the generated workflows end up
templates - Partial SCOPE files for custom template system
direct - Full SCOPE files that don't need fix-ups; directly copied to output
GenTel.py - Python script that contains the logic needed to generate output



Useful commands:

Export the current workflow state over your local one, so you can check for any
uncommitted changes:
.\XflowConfig.exe ExportWorkflow -xflowServiceUrl https://wfm-data.corp.microsoft.com/xflow/service/ -name ChakraDaily -outputDir <ChakraFull root>\tools\Telemetry\output\

Import your new workflow set-up over the remote one, making it live on the next
set of jobs to be kicked off:

