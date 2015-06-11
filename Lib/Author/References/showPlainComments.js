
(function () {
    var MAX_DESCRIPTION_LEN = 500;
    //
    //  Extension events. 
    //  Copy plain comments into description property if available.
    //  Do not copy if VS doc comments are there.
    //
    intellisense.addEventListener('statementcompletionhint', function (event) {
        if (event.symbolHelp.description) return;
        var itemValue = event.completionItem.value;
        if (typeof itemValue === "function") {
            var functionHelp = event.symbolHelp.functionHelp;
            if (!canApplyComments(functionHelp)) return;
            var comments = intellisense.getFunctionComments(itemValue);
            comments.above = comments.above || event.completionItem.comments;
            applyComments(functionHelp, comments);
        } else {
            var comments = event.completionItem.comments;
            if (isDocComment(comments)) return;
            setDescription(event.symbolHelp, comments);
        }
    });
    intellisense.addEventListener('signaturehelp', function (event) {
        if (!canApplyComments(event.functionHelp)) return;
        applyComments(event.functionHelp, event.functionComments);
    });
    //
    //  Helpers
    //
    function applyComments(functionHelp, comments) {
        var signatures = functionHelp.signatures;
        var signature = signatures[0];
        // Do not apply if VS doc comments were applied
        if (!canApplyComments(functionHelp)) return;
        // Do not apply VS doc comments
        if (comments.insideIsDoc) return;
        if (comments.aboveIsDoc) return;
        if (comments.above && !comments.aboveIsDoc) {
            setDescription(signature, comments.above);
        }
        // Populate parameters descriptions
        signature.params.forEach(function (param, index) {
            var paramComment = comments.paramComments[index];
            if (!paramComment.comment) return;
            if (paramComment.name != param.name) return false;
            setDescription(param, paramComment.comment);
        });
    }
    function canApplyComments(functionHelp) {
        var signatures = functionHelp.signatures;
        var signature = signatures[0];
        if (signatures.length > 1) return false;
        if (signature.description) return false;
        if (!signature.params.every(function (param) { return !param.description; })) return false;
        return true;
    }
    function isDocComment(comment) {
        // Simple heuristic to detect xml doc comments.
        return !!(comment && comment.charAt(0) === '<');
    }
    function setDescription(o, text) {
        // Trim description if needed
        text = (text && text.length > MAX_DESCRIPTION_LEN) ? text.substring(0, MAX_DESCRIPTION_LEN) + '...' : text;
        // Encode characters to accomodate markup as well as new lines
        text = text.replace(/&(?!#?\w+;)/g, "&amp;")
            .replace(/</g, "&lt;")
            .replace(/>/g, "&gt;")
            .replace(/(\r\n|\n|\r)/gm, "<br/>");
        o.description = text;
    }
})();