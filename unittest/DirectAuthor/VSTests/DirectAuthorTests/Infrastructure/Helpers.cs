using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using Microsoft.BPT.Tests.DirectAuthor;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace DirectAuthorTests
{
    public static class Helpers2
    {
        public static void AppendAsLiteral(this StringBuilder builder, string value)
        {
            if (value == null)
                builder.Append("(string)null");
            else if (value.Any(c => !char.IsLetterOrDigit(c) && !char.IsWhiteSpace(c) && (!char.IsPunctuation(c) || c == '"')))
            {
                builder.Append("@\"");
                foreach (var ch in value)
                    switch (ch)
                    {
                        case '"': builder.Append("\\\""); break;
                        default: builder.Append(ch); break;
                    }
                builder.Append('"');
            }
            else
            {
                builder.Append('"');
                builder.Append(value);
                builder.Append('"');
            }
        }

        public static void Expect<T>(this T value, T expect)
        {
            Assert.AreEqual(expect, value);
        }

        public static void Expect(this string actual, string expected)
        {
            if (actual == null)
            {
                if (expected != null)
                {
                    Assert.Fail("Expected value: " + expected, " actual value: null");
                }
                return;
            }
            else if (expected == null)
            {
                Assert.Fail("Expected value: null, actual value: " + actual);
            }


            Action<int> Mismatch = (pos) =>
            {
                Assert.Fail(String.Format("Mismatch at position {0}, expected: '{1}...', actual: '{2}...'",
                    pos.ToString(System.Globalization.CultureInfo.CurrentCulture),
                    expected.Substring(pos, Math.Min(40, expected.Length - pos)),
                    actual.Substring(pos, Math.Min(40, actual.Length - pos))));
            };

            int len = Math.Min(expected.Length, actual.Length);

            for (int i = 0; i < len; i++)
            {
                if (expected[i] != actual[i])
                {
                    Mismatch(i);
                }
            }

            if (expected.Length != actual.Length)
            {
                Mismatch(len);
            }
        }

        public static IEnumerable<T> Concat<T>(this IEnumerable<T> items, T item1)
        {
            foreach (var item in items) yield return item;
            yield return item1;
        }

        public static IEnumerable<T> Concat<T>(this IEnumerable<T> items, T item1, T item2)
        {
            foreach (var item in items) yield return item;
            yield return item1;
            yield return item2;
        }

        public static IEnumerable<T> Concat<T>(this IEnumerable<T> items, T item1, T item2, params T[] additional)
        {
            foreach (var item in items) yield return item;
            yield return item1;
            yield return item2;
            foreach (var item in additional) yield return item;
        }

        public static string CompactJSON(this string jsonString)
        {
            StringBuilder compacted = new StringBuilder();
            bool inValue = false;

            foreach (char ch in jsonString)
            {
                switch (ch)
                {
                    case '\"':
                        inValue = !inValue;
                        compacted.Append(ch);
                        break;
                    case ' ':
                    case '\t':
                        if (inValue)
                            compacted.Append(ch);
                        break;
                    case '\n':
                    case '\r':
                        break;
                    default:
                        compacted.Append(ch);
                        break;
                }
            }
            return compacted.ToString();
        }

        public static string RemoveNewLines(this string inputString)
        {
            return inputString.Replace("\n", "").Replace("\r", "");
        }

        public static void Append(this IAuthorTestFile file, string text)
        {
            file.InsertText(file.Text.Length, text);
        }

        public static void Touch(this IAuthorTestFile file)
        {
            var offset = file.Text.Length;
            file.InsertText(offset, " ");
            file.DeleteText(offset, 1);
        }
    }
}
