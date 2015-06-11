using System.Text.RegularExpressions;
using Microsoft.VisualStudio.JavaScript.LanguageService.Engine;

namespace Microsoft.BPT.Tests.DirectAuthor
{
    public interface IAuthorTestFile
    {
        string Name { get; }
        string Text { get; }

        void InsertText(int offset, string text);
        string DeleteText(int offset, int length);
        string Replace(int offset, int length, string text);

        Match Match(Regex regex);
        Match Match(int offset, Regex regex);
       
        int OffsetOf(Regex regex);
        int OffsetOf(string text);
        int OffsetAfter(Regex regex);
        int OffsetAfter(string text);

        IAuthorFileHandle GetHandle();
     }
}
