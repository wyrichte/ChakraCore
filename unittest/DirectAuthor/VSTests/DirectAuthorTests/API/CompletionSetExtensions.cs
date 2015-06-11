using System.Collections.Generic;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace Microsoft.VisualStudio.JavaScript.LanguageService.Engine
{
    public static class CompletionSetExtensions
    {
        public static AuthorCompletion Item(this IAuthorCompletionSet completions, string name)
        {
            return completions.ToEnumerable().Item(name);
        }

        public static AuthorCompletion Item(this IEnumerable<AuthorCompletion> completions, string name)
        {
            int entryIndex = -1;
            var item = GetItem(completions, name, out entryIndex);
            Assert.IsTrue(item != null);
            return item.Value;
        }

        public static IAuthorSymbolHelp GetHintFor(this IAuthorCompletionSet completions, string name)
        {
            Assert.IsNotNull(completions);

            int entryIndex = -1;
            GetItem(completions.ToEnumerable(), name, out entryIndex);     
            Assert.IsTrue(entryIndex >= 0);

            return completions.GetHintFor(entryIndex);
        }

        private static AuthorCompletion? GetItem(IEnumerable<AuthorCompletion> completions, string name, out int entryIndex)
        {
            int resultIndex = -1;
            AuthorCompletion? result = null;
            var completion = completions.Where((c, index) =>
            {
                if (c.DisplayText == name)
                {
                    resultIndex = index;
                    result = c;
                }
                return false;
            }).SingleOrDefault();

            entryIndex = resultIndex;
            return result;
        }
    }
}
