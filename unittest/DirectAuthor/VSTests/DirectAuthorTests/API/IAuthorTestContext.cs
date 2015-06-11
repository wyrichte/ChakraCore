using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;

using Microsoft.VisualStudio.JavaScript.LanguageService.Engine;

namespace Microsoft.BPT.Tests.DirectAuthor
{
    public interface IAuthorTestContext
    {
        IAuthorTestFile PrimaryFile { get; }
        event Action<AuthorFileAuthoringPhase, int> PhaseChanged;

        void AddContextFiles(params IAuthorTestFile[] files);
        void RemoveContextFiles(params IAuthorTestFile[] files);
        void Update();
        void Update(out Int64 callTime);
        void Hurry(int phaseId);
        string GetASTAsJSON();
        string GetASTAsJSON(out Int64 callTime);
        IAuthorParseNodeCursor GetASTCursor();
        IAuthorParseNodeCursor GetASTCursor(out Int64 callTime);
        IAuthorRegionSet GetRegions();
        IAuthorRegionSet GetRegions(out Int64 callTime);
        IAuthorMessageSet GetMessages();
        IAuthorMessageSet GetMessages(out Int64 callTime);
        IAuthorCompletionSet GetCompletionsAt(int offset, AuthorCompletionFlags flags = AuthorCompletionFlags.acfMembersFilter);
        IAuthorCompletionSet GetCompletionsAt(int offset, AuthorCompletionFlags flags , out Int64 callTime);
        AuthorParameterHelp GetImplicitParameterHelpAt(int offset);
        AuthorParameterHelp GetParameterHelpAt(int offset);
        AuthorParameterHelp GetParameterHelpAt(int offset, out Int64 callTime);
        IAuthorSymbolHelp GetQuickInfoAt(int offset);
        void GetDefinitionLocation(int offset, out IAuthorTestFile file, out int location);
        IAuthorReferenceSet GetReferences(int offset);

        IEnumerable<string> LoggedMessages { get; }

        void AddAsyncScript(string url, IAuthorTestFile file);
        void TakeOwnership(object o);
        void Close();
        IAuthorTestFile GetFileByText(string text);
        IAuthorDiagnostics GetDiagnostics();
        IAuthorStructure GetStructure();
    }
}