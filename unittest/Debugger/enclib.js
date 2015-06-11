var enc = (function () {

    var space = "                        ";
    function editKindString(kind) {
        return kind + space.substr(0, 8 - kind.length);
    }

    var default_options = {
        match: true, // dump tree match
        edits: true, // dump edit script
    };

    return {
        astDiff: function (before, after, options) {

            function nodeString(node, source) {
                var len = Math.min(node.lim - node.min, 16);
                return node.type + "@" + node.min + " " + source.substr(node.min, len);
            }

            before = String(before);
            after = String(after);
            options = options || default_options;

            var diff = EditTest.AstDiff(before, after, options.full);
            var result = {};

            // Convert Match
            if (options.match) {
                var str = JSON.stringify(diff.match, function (key, value) {
                    if (Array.isArray(value) && value.length == 2) { // A pair of matched nodes
                        return [
                            nodeString(value[0], before),
                            nodeString(value[1], after)];
                    }
                    return value;
                });

                result.Match = JSON.parse(str);
            }

            // Convert Edits
            if (options.edits) {
                var str = JSON.stringify(diff.edits, function (key, value) {
                    if (Array.isArray(this)) {
                        switch (value.kind) {
                            case "Delete":
                                value = editKindString(value.kind) + nodeString(value.oldNode, before);
                                break;
                            case "Insert":
                                value = editKindString(value.kind) + nodeString(value.newNode, after);
                                break;
                            case "Update":
                            case "Move":
                            case "Reorder":
                                value = editKindString(value.kind) + nodeString(value.oldNode, before) + " -> " + nodeString(value.newNode, after);
                                break;
                        }
                    }
                    return value;
                });

                result.Edits = JSON.parse(str);
            }

            return result;
        }
    };

})();
