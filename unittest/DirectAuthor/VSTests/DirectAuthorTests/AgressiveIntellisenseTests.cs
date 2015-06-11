using System;
using System.Text;
using System.Collections.Generic;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.IO;
using Microsoft.VisualStudio.JavaScript.LanguageService.Engine;
using Microsoft.BPT.Tests.DirectAuthor;

namespace DirectAuthorTests
{
    [TestClass]
    public class AgressiveIntellisenseTests : CompletionsBase
    {
        internal class Ranges
        {
            struct Range { public int start; public int end;};

            private List<Range> ranges;

            public Ranges()
            {
                ranges = new List<Range>();
            }

            public void AddRange(int start, int end)
            {
                Assert.IsTrue(end >= start);

                this.ranges.Add(new Range() { start = start, end = end });
            }

            public bool InRange(int offset)
            {
                foreach (var range in this.ranges)
                {
                    if (offset >= range.start && offset <= range.end)
                    {
                        return true;
                    }
                }
                return false;
            }
        }

        protected void PerformHurriedAgressiveCompletionRequest(string text)
        {
            WithMTASession(() =>
            {
                PerformAgressiveCompletionRequest(text);
            });
        }

        protected void PerformAgressiveCompletionRequest(string text)
        {
            var offsets = ParseRequests(text);

            // find the ranges where completion is supported
            Ranges onRanges = new Ranges();
            int startOffset = 0;
            bool currentRangeIsOn = true;
            foreach (var request in offsets.Requests)
            {
                switch (request.Data)
                {
                    case "on":
                        if (!currentRangeIsOn)
                        {
                            startOffset = request.Offset;
                            currentRangeIsOn = true;
                        }
                        break;
                    case "off":
                        if (currentRangeIsOn)
                        {
                            onRanges.AddRange(startOffset, request.Offset - 1);
                            currentRangeIsOn = false;
                        }
                        break;
                    default:
                        Assert.Fail("Unsupported value: " + request.Data);
                        break;
                }
            }
            if (currentRangeIsOn && startOffset > 0)
            {
                onRanges.AddRange(startOffset, offsets.Text.Length + 1);
            }

            // initialize the context with an empty file
            var file = _session.FileFromText(String.Empty);
            var context = _session.OpenContext(file);

            // split the text into tokens and add one token at a time
            bool errorEncountered = false;
            var tokens = offsets.Text.Split(' ');

            // try the first offset in the file
            errorEncountered |= VerifyCompletionSupported(context, file, onRanges.InRange(file.Text.Length), tokens != null ? tokens[0] : null);

            for(int i = 0; i < tokens.Length; i++)
            {
                var token = tokens[i];
                var nextToken = i < tokens.Length - 1 ? tokens[i + 1] : null;

                // insert the next token
                file.InsertText(file.Text.Length, token);

                // insert a space
                file.InsertText(file.Text.Length, " ");
                errorEncountered |= VerifyCompletionSupported(context, file, onRanges.InRange(file.Text.Length), nextToken);
            }
            Assert.IsFalse(errorEncountered);
        }

        bool IsText(string value)
        {
            return (!String.IsNullOrEmpty(value) && ((value[0] >= 'a' && value[0] <= 'z') || (value[0] >= 'A' && value[0] <= 'Z') || value[0] == '_'));
        }

        bool IsEmptyCompletion(IAuthorCompletionSet completion)
        {
            return completion == null || completion.Count == 0;
        }

        bool VerifyCompletionSupported(IAuthorTestContext context, IAuthorTestFile file, bool expectCompletion, string nextToken)
        {
            bool debug = false;
            bool errorEncountered = false;
            var completionFlags = AuthorCompletionFlags.acfMembersFilter | AuthorCompletionFlags.acfSyntaxElementsFilter;

            IAuthorCompletionSet completions = null;

            // call getCompletion with hurry enabled
            using (IDisposable hurry = ExecutionLimiter(context))
            {
                try
                {
                    completions = context.GetCompletionsAt(file.Text.Length, completionFlags);
                }
                catch (Exception)
                {
                    throw;
                }
            }

            if (expectCompletion && IsEmptyCompletion(completions))
            {
                Console.WriteLine("Completion at offset: {0} is NOT supported. It was expected to.", file.Text.Length);
                errorEncountered = true;
            }
            else if (!expectCompletion && !IsEmptyCompletion(completions))
            {
                Console.WriteLine("Completion at offset: {0} is supported. It was NOT expected to.", file.Text.Length);
                errorEncountered = true;
            }
            else if (expectCompletion && !IsEmptyCompletion(completions))
            {
                if (debug)
                    Console.WriteLine("OK -- ON");

                if (!String.IsNullOrEmpty(nextToken) && IsText(nextToken))
                {
                    if (!completions.ToEnumerable().Names().Contains(nextToken))
                    {
                        Console.WriteLine("\"{0}\" -- was not found in completion", nextToken);
                        errorEncountered = true;
                    }
                    else
                    {
                        if (debug)
                            Console.WriteLine("\"{0}\" -- Found", nextToken);
                    }
                }
            }
            else if (!expectCompletion)
            {
                if (debug)
                    Console.WriteLine("OK -- OFF");
            }

            if (debug || errorEncountered)
            {
                Console.WriteLine("-----------");
                Console.WriteLine(file.Text + "|");
                Console.WriteLine("-----------");
                Console.WriteLine();
            }

            return errorEncountered;
        }

        [TestMethod]
        public void AgressiveCompletion()
        {
            PerformHurriedAgressiveCompletionRequest(@"var|off| x|on| = 0 ; 
function|off| f ( a , b , c , d )|on| { return 0 ; } 
try { x = x + x ; } catch|off| ( exception ) {|on| exception ( ) ; } finally { x --; } 
while ( x > 0 && x < 3 ) { x ++ } 
do { x = x + 1 ; f ( x ) ;} while ( x ) ; 
if ( x > x ) x = x - x ; else x = 0 ;
for ( x = 0 ; x < x ; x ++ ) { x = x + 1 ; }
for ( var|off| i|on| = 0 ; i < x ; i ++ ) { x = i ; }
for ( var|off| z|on| in [ 0 , 1 , 2 ] ) x = z + z ;
switch ( x ) { case 0 : x = 0; break ; default : break ; }
with ( x ) { x ++ ; }
debugger ;
x = new Object ( ) ;
y =|on| {|off| a :|on| x|off| , b :|on| function|off| ( ){|on| }|off| }|on| ;
( {|off| a :|on| y|off| , b :|on| 3|off| , get c ( ){|on| }|off|, set c ( v ){|on| }|off| } )|on|
/*|off| in a comment */|on| //|off| in another coment 
|on| ");

        }
    }
}
