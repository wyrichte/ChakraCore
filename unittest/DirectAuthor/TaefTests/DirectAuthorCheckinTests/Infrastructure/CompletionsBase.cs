using System.Collections.Generic;
using System.Linq;
using Microsoft.VisualStudio.JavaScript.LanguageService.Engine;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace DirectAuthorCheckinTests
{
    public class CompletionsBase : DirectAuthorTest
    {
        internal IEnumerable<IAuthorCompletionSet> CollectCompletions(string text, AuthorCompletionFlags flags = AuthorCompletionFlags.acfMembersFilter)
        {
            var requests = new List<IAuthorCompletionSet>();
            PerformRequests(text, (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset, flags);
                requests.Add(completions);
            });
            return requests.ToArray();
        }

        internal void ValidateNoCompletion(string js, AuthorCompletionFlags flags = AuthorCompletionFlags.acfMembersFilter, params string[] contextFiles)
        {
            PerformRequests(js, (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset, flags);
                Assert.IsNull(completions, "no completion results expected: " + js);
            }, contextFiles);
        }

        internal void ValidateHasCompletions(string js, params string[] completionsToVerify)
        {
            PerformCompletionRequests(js, (completions, data, i) =>
            {
                completions.ExpectContains(completionsToVerify);
            });
        }

        internal void ValidateExcludesCompletions(string js, params string[] completionsExcluded)
        {
            PerformCompletionRequests(js, (completions, data, i) => completions.ExpectNotContains(completionsExcluded));
        }
    }

    public static class CompletionHelpers
    {
        private static string FindMissing(this IEnumerable<AuthorCompletion> completions, IEnumerable<string> expect)
        {
            var dictionary = completions.Names().ToDictionary(v => v);
            return expect.FirstOrDefault(v => !dictionary.ContainsKey(v));
        }

        public static bool Contains(this IEnumerable<AuthorCompletion> completions, IEnumerable<string> expect)
        {
            return FindMissing(completions, expect) == null;
        }

        public static bool Contains(this IEnumerable<AuthorCompletion> completions, string one, params string[] others)
        {
            return Contains(completions, Enumerable.Repeat(one, 1).Concat(others));
        }

        public static void ExpectContains(this IEnumerable<AuthorCompletion> completions, IEnumerable<string> expect)
        {
            var missing = FindMissing(completions, expect);
            Assert.IsNull(missing, "Expected completion list to contain '" + missing + "'");
        }

        public static void ExpectContains(this IEnumerable<AuthorCompletion> completions, string one, params string[] others)
        {
            ExpectContains(completions, Enumerable.Repeat(one, 1).Concat(others));
        }

        public static void ExpectNotContains(this IEnumerable<AuthorCompletion> completions, IEnumerable<string> expect)
        {
            var dictionary = completions.Names().ToDictionary(v => v);
            var found = expect.FirstOrDefault(v => dictionary.ContainsKey(v));
            Assert.IsNull(found, "Expected completion list not to contain '" + found + "'");
        }

        public static void ExpectNotContains(this IEnumerable<AuthorCompletion> completions, string one, params string[] others)
        {
            ExpectNotContains(completions, Enumerable.Repeat(one, 1).Concat(others));
        }

        public static IEnumerable<string> Names(this IEnumerable<AuthorCompletion> completions)
        {
            return completions.Select(c => c.Name);
        }
    }
}
