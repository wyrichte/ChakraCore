using System;
using System.Collections.Generic;

namespace Microsoft.VisualStudio.JavaScript.LanguageService.Engine
{
    public static class AuthorHelpers
    {
        interface ISetPolicy<T, E>
        {
            int GetCount(T set);
            void GetItems(T set, int start, int count, E[] dest);
        }

        struct AuthorCompletionSetPolicy : ISetPolicy<IAuthorCompletionSet, AuthorCompletion>
        {
            public int GetCount(IAuthorCompletionSet set) { return set.Count; }
            public void GetItems(IAuthorCompletionSet set, int start, int count, AuthorCompletion[] dest) { set.GetItems(start, count, dest); }
        }

        struct AuthorMessageSetPolicy : ISetPolicy<IAuthorMessageSet, AuthorFileMessage>
        {
            public int GetCount(IAuthorMessageSet set) { return set.Count; }
            public void GetItems(IAuthorMessageSet set, int start, int count, AuthorFileMessage[] dest) { set.GetItems(start, count, dest); }
        }

        struct AuthorRegionSetPolicy : ISetPolicy<IAuthorRegionSet, AuthorFileRegion>
        {
            public int GetCount(IAuthorRegionSet set) { return set.Count; }
            public void GetItems(IAuthorRegionSet set, int start, int count, AuthorFileRegion[] dest) { set.GetItems(start, count, dest); }
        }

        struct AuthorParameterSetPolicy : ISetPolicy<IAuthorParameterSet, IAuthorParameter>
        {
            public int GetCount(IAuthorParameterSet set) { return set.Count; }
            public void GetItems(IAuthorParameterSet set, int start, int count, IAuthorParameter[] dest) { set.GetItems(start, count, dest); }
        }

        struct AuthorSignatureSetPolicy : ISetPolicy<IAuthorSignatureSet, IAuthorSignature>
        {
            public int GetCount(IAuthorSignatureSet set) { return set.Count; }
            public void GetItems(IAuthorSignatureSet set, int start, int count, IAuthorSignature[] dest) { set.GetItems(start, count, dest); }
        }

        struct AuthorParseNodeSetPolicy : ISetPolicy<IAuthorParseNodeSet, AuthorParseNode>
        {
            public int GetCount(IAuthorParseNodeSet set) { return set.Count; }
            public void GetItems(IAuthorParseNodeSet set, int start, int count, AuthorParseNode[] dest) { set.GetItems(start, count, dest); }
        }

        struct AuthorAllocInfoSetPolicy : ISetPolicy<IAuthorAllocInfoSet, IAuthorAllocInfo>
        {
            public int GetCount(IAuthorAllocInfoSet set) { return set.Count; }
            public void GetItems(IAuthorAllocInfoSet set, int start, int count, IAuthorAllocInfo[] dest) { set.GetItems(start, count, dest); }
        }

        struct AuthorStructureNodeSetPolicy : ISetPolicy<IAuthorStructureNodeSet, AuthorStructureNode>
        {
            public int GetCount(IAuthorStructureNodeSet set) { return set.Count; }
            public void GetItems(IAuthorStructureNodeSet set, int start, int count, AuthorStructureNode[] dest) { set.GetItems(start, count, dest); }
        }

        struct AuthorReferenceSetPolicy : ISetPolicy<IAuthorReferenceSet, AuthorSymbolReference>
        {
            public int GetCount(IAuthorReferenceSet set) { return set.Count; }
            public void GetItems(IAuthorReferenceSet set, int start, int count, AuthorSymbolReference[] dest) { set.GetItems(start, count, dest); }
        }

        struct AuthorCompatibleWithSetPolicy : ISetPolicy<IAuthorCompatibleWithSet, IAuthorCompatibleWithInfo>
        {
            public int GetCount(IAuthorCompatibleWithSet set) { return set.Count; }
            public void GetItems(IAuthorCompatibleWithSet set, int start, int count, IAuthorCompatibleWithInfo[] dest) { set.GetItems(start, count, dest); }
        }

        static class ToEnumerableImpl<P, T, E> where P : struct, ISetPolicy<T, E>
        {
#pragma warning disable 0649
            static P MyP;
#pragma warning restore 0649

            const int pageSize = 100;

            public static IEnumerable<E> ToEnumerable(T set)
            {
                var count = MyP.GetCount(set);
                if (count < pageSize)
                {
                    var elements = new E[count];
                    MyP.GetItems(set, 0, count, elements);
                    return elements;
                }
                return ToPagedEnumerable(set);
            }

            private static IEnumerable<E> ToPagedEnumerable(T set)
            {
                var count = MyP.GetCount(set);
                var start = 0;
                while (count > 0)
                {
                    var len = Math.Min(count, pageSize);
                    var elements = new E[len];
                    MyP.GetItems(set, start, len, elements);
                    foreach (var element in elements) yield return element;
                    count -= len;
                    start += len;
                }
            }
        }

        public static IEnumerable<AuthorCompletion> ToEnumerable(this IAuthorCompletionSet set) { return ToEnumerableImpl<AuthorCompletionSetPolicy, IAuthorCompletionSet, AuthorCompletion>.ToEnumerable(set); }
        public static IEnumerable<AuthorFileMessage> ToEnumerable(this IAuthorMessageSet set) { return ToEnumerableImpl<AuthorMessageSetPolicy, IAuthorMessageSet, AuthorFileMessage>.ToEnumerable(set); }
        public static IEnumerable<AuthorFileRegion> ToEnumerable(this IAuthorRegionSet set) { return ToEnumerableImpl<AuthorRegionSetPolicy, IAuthorRegionSet, AuthorFileRegion>.ToEnumerable(set); }
        public static IEnumerable<IAuthorParameter> ToEnumerable(this IAuthorParameterSet set) { return ToEnumerableImpl<AuthorParameterSetPolicy, IAuthorParameterSet, IAuthorParameter>.ToEnumerable(set); }
        public static IEnumerable<IAuthorSignature> ToEnumerable(this IAuthorSignatureSet set) { return ToEnumerableImpl<AuthorSignatureSetPolicy, IAuthorSignatureSet, IAuthorSignature>.ToEnumerable(set); }
        public static IEnumerable<AuthorParseNode> ToEnumerable(this IAuthorParseNodeSet set) { return ToEnumerableImpl<AuthorParseNodeSetPolicy, IAuthorParseNodeSet, AuthorParseNode>.ToEnumerable(set); }
        public static IEnumerable<IAuthorAllocInfo> ToEnumerable(this IAuthorAllocInfoSet set) { return ToEnumerableImpl<AuthorAllocInfoSetPolicy, IAuthorAllocInfoSet, IAuthorAllocInfo>.ToEnumerable(set); }
        public static IEnumerable<AuthorStructureNode> ToEnumerable(this IAuthorStructureNodeSet set) { return ToEnumerableImpl<AuthorStructureNodeSetPolicy, IAuthorStructureNodeSet, AuthorStructureNode>.ToEnumerable(set); }
        public static IEnumerable<AuthorSymbolReference> ToEnumerable(this IAuthorReferenceSet set) { return ToEnumerableImpl<AuthorReferenceSetPolicy, IAuthorReferenceSet, AuthorSymbolReference>.ToEnumerable(set); }
        public static IEnumerable<IAuthorCompatibleWithInfo> ToEnumerable(this IAuthorCompatibleWithSet set) { return ToEnumerableImpl<AuthorCompatibleWithSetPolicy, IAuthorCompatibleWithSet, IAuthorCompatibleWithInfo>.ToEnumerable(set); }
    }

}