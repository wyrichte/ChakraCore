﻿using Microsoft.VisualStudio.JavaScript.LanguageService.Engine;

namespace Microsoft.BPT.Tests.DirectAuthor
{
    public interface IAuthorTestSession
    {
        void Close();
        IAuthorTestFile ReadFile(string filename);
        IAuthorTestFile FileFromText(string text, string name = null);
        IAuthorTestContext OpenContext(IAuthorTestFile primaryFile, params IAuthorTestFile[] contextFiles);
        IAuthorTestContext OpenContext(IAuthorTestFile primaryFile, AuthorHostType hostType, params IAuthorTestFile[] contextFiles);
        IAuthorTestContext OpenContext(IAuthorTestFile primaryFile, AuthorHostType hostType, AuthorScriptVersion scriptVersion, params IAuthorTestFile[] contextFiles);
        Microsoft.VisualStudio.JavaScript.LanguageService.Engine.IAuthorColorizeText GetColorizer();
        IAuthorFileHandle TakeOwnershipOf(IAuthorTestFile file);
        void Cleanup(bool exhaustive = false);
    }
}