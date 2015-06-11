using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

using Microsoft.BPT.Tests.DirectAuthor;
using Microsoft.VisualStudio.JavaScript.LanguageService.Engine;
using Microsoft.VisualStudio.TestTools.UnitTesting;

using System.IO;

namespace DirectAuthorTests
{
    [TestClass]
    public class JQueryBase : CompletionsBase
    {
        protected static readonly string JQueryFilePath = Path.Combine(Paths.FilesPath, @"JQuery\jquery-1.7.js");

        private static bool IsKeyword(AuthorTokenColorInfo token)
        {
            return (AuthorTokenKind.atkBreak <= token.Kind && token.Kind <= AuthorTokenKind.atkWith);
        }

        private enum ScannerState
        {
            Valid,
            InVarDeclaration,
            InVarInitialization,
            InFunctionDeclaration,
            InCatchDefinition,
        }

        private AuthorTokenColorInfo[] GetValidCompletionTokens(string text, bool includeKeywords = true)
        {
            List<AuthorTokenColorInfo> tokens = new List<AuthorTokenColorInfo>();
            var colorizer = _session.GetColorizer();
            var colorization = colorizer.Colorize(text, text.Length, AuthorSourceState.SOURCE_STATE_INITIAL);
            AuthorTokenColorInfo token;
            ScannerState state = ScannerState.Valid;
            AuthorTokenKind prevTokenKind = AuthorTokenKind.atkEnd;
            AuthorTokenKind prev2TokenKind = AuthorTokenKind.atkEnd;
            int containerCount = 0;

            do
            {
                token = colorization.Next();
                if ((state == ScannerState.Valid || state == ScannerState.InVarInitialization) && ((includeKeywords && IsKeyword(token)) || token.Kind == AuthorTokenKind.atkIdentifier))
                    tokens.Add(token);
                switch (token.Kind)
                {
                    case AuthorTokenKind.atkVar:
                        state = ScannerState.InVarDeclaration;
                        break;
                    case AuthorTokenKind.atkAsg:
                        if (state == ScannerState.InVarDeclaration)
                        {
                            state = ScannerState.InVarInitialization;
                            containerCount = 0;
                        }
                        break;
                    case AuthorTokenKind.atkFunction:
                        state = ScannerState.InFunctionDeclaration;
                        break;
                    case AuthorTokenKind.atkCatch:
                        state = ScannerState.InCatchDefinition;
                        break;
                    case AuthorTokenKind.atkLCurly:
                        if (state == ScannerState.InCatchDefinition || state == ScannerState.InFunctionDeclaration)
                            state = ScannerState.Valid;
                        containerCount++;
                        break;
                    case AuthorTokenKind.atkSColon:
                        if (state == ScannerState.InVarDeclaration)
                            state = ScannerState.Valid;
                        if (state == ScannerState.InVarInitialization && containerCount == 0)
                            state = ScannerState.Valid;
                        break;
                    case AuthorTokenKind.atkColon:
                        // remove the last identifier 
                        if (tokens.Count > 0 && tokens[tokens.Count - 1].Kind == AuthorTokenKind.atkIdentifier && prev2TokenKind != AuthorTokenKind.atkQMark && prevTokenKind == AuthorTokenKind.atkIdentifier)
                            tokens.RemoveAt(tokens.Count - 1);
                        break;
                    case AuthorTokenKind.atkLBrack:
                    case AuthorTokenKind.atkLParen:
                        containerCount++;
                        break;
                    case AuthorTokenKind.atkRCurly:
                    case AuthorTokenKind.atkRBrack:
                    case AuthorTokenKind.atkRParen:
                        containerCount--;
                        break;
                    case AuthorTokenKind.atkComma:
                        if (state == ScannerState.InVarInitialization && containerCount == 0)
                            state = ScannerState.InVarDeclaration;
                        break;
                }

                prev2TokenKind = prevTokenKind;
                prevTokenKind = token.Kind;
            }
            while (token.Kind != AuthorTokenKind.atkEnd);
            Marshal.ReleaseComObject(colorization);
            Marshal.ReleaseComObject(colorizer);

            return tokens.ToArray();
        }

        protected void PerformJQueryTest(string text, params string[] missingCompletions)
        {
            if (_session != null)
            {
                _session.Close();
                _session = null;
            }

            List<string> ignoredCompletions = new List<string>(missingCompletions);

            WithMTASession(() =>
            {
                var primaryFile = _session.FileFromText(text);
                var contextFiles = new[] {
                        _session.ReadFile(Paths.LibHelpPath),
                        _session.ReadFile(Paths.SiteTypesWebPath),
                        _session.ReadFile(Paths.DomWebPath),
                        _session.ReadFile(JQueryFilePath)
                };
                var context = _session.OpenContext(primaryFile, AuthorHostType.ahtBrowser, contextFiles);

                AuthorTokenColorInfo[] tokens = GetValidCompletionTokens(primaryFile.Text, false);

                bool foundAllCompletions = true;

               using (IDisposable hurry = ExecutionLimiter(context))
               {
                    foreach (var token in tokens)
                    {
                        AuthorCompletionFlags flags = IsKeyword(token) ? AuthorCompletionFlags.acfSyntaxElementsFilter : AuthorCompletionFlags.acfMembersFilter;
                        var tokenText = primaryFile.Text.Substring(token.StartPosition, token.EndPosition - token.StartPosition);

                        bool found = false;
                        var completion = context.GetCompletionsAt(token.StartPosition, flags);

                        if (completion != null && completion.Count > 0)
                        {
                            found = completion.ToEnumerable().Contains(tokenText);
                        }

                        if (completion != null)
                        {
                            Marshal.ReleaseComObject(completion);
                            completion = null;
                        }

                        if (!found)
                        {
                            Console.Write("===> Could not find token: " + tokenText);
                            if (!ignoredCompletions.Contains(tokenText))
                            {
                                foundAllCompletions = false;
                            }
                            else
                            {
                                ignoredCompletions.Remove(tokenText);
                                Console.WriteLine("  [Ignored]");
                            }
                            Console.WriteLine(DumpObject(token));
                        }
                        else
                        {
                            Console.WriteLine("Found token: " + tokenText);
                        }
                    }
                }

                Assert.IsTrue(foundAllCompletions, "Missed at least 1 completion!!");
           });
        }
    }
}
