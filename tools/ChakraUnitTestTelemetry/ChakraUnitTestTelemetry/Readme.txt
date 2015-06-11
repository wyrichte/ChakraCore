ChakraUnitTestTelemetry Prerequisites

The ChakraUnitTestTelemetry project is set to automatically run StyleCop, Code Analysis, and utilizes Code Contracts
when building.  You'll need to install these prerequisites in order to build the project.

StyleCop (Last version built against was 4.7)
http://stylecop.codeplex.com/

Code Contracts (Last version built against was 1.5.60502.11)
http://visualstudiogallery.msdn.microsoft.com/1ec7db13-3363-46c9-851f-1ce455f66970#5

Optionally, you can install the code contracts extensions for intellisense as well:
http://visualstudiogallery.msdn.microsoft.com/02de7066-b6ca-42b3-8b3c-2562c7fa024f

When an unhandled exception occurs in the tool, traces of the issue are dumped to \\bpt-scratch\UserFiles\cmorse\Tools\ChakraUnitTestTelemetry\Unhandled Exceptions.
