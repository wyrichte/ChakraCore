
<!-- スライド画像 jquery -->

	<!-- メインスライド -->

function f() {
    // Blue 631601: CAS:WebCrawler: ASSERT:  pLastReuseFunc->LengthInChars() == pnodeParent->LengthInCodepoints()
    //
    //  This test file contains unicode characters in UTF8 to repro conflicting m_cMultUnit.
    //
    //  This file contains HTMLCommentSuffix "// ... -->". Trident will tell us to trim it from source.
    //  However, Scanner didn't enforce that, and may actually scan it and report it to Parser. But at the same
    //  time m_cMultUnit counting enforces it and wouldn't count any multi-unit chars. This results in mismatching
    //  cbLength and cchLength.
}

f();

// center_column タブ処理 -->

