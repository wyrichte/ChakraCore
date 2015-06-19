//
// Copyright (C) Microsoft. All rights reserved.
//

(function(window, undefined) {

    var editor;

    function MonacoSetup() {
        // You don't need to know that the HTML language mode is located at vs/languages/html/html,
        // you can use the modes registry and simply use a mime type

        // Here is the list of supported mime types:
        // "text/plain", "text/x-coffeescript", "text/coffeescript", "text/css", "text/x-cpp", "text/x-handlebars-template",
        // "text/html", "text/x-java-source", "text/javascript", "application/json", "text/x-web-markdown", "text/x-csharp",
        // "text/x-cshtml", "application/x-php", "text/x-vb", "text/xml", "application/xml", "application/xaml+xml", "text/x-bat"
        Monaco.Editor.getOrCreateMode('text/javascript').then(function() {
            editor = Monaco.Editor.create(document.getElementById("editor"), {
                value: "",
                mode: "text/javascript"
            });
        });
    }

    function setUpUI() {
        require.config({
            baseUrl: "res://" + window.location.host + "/23/debugger/editor"
        });

        require(["vs/editor/editor.main"], MonacoSetup);
    }

    function IsEditorInit() {
        if (editor === undefined) {
            console.log("warning: editor not initialized");
            return false;
        }
        return true;
    }

    //
    // external functions
    //

    function SetMonacoCode(code) {
        if (IsEditorInit()) {
            editor.getModel().setValue(code);
        }
    }

    function RefreshEditorLayout() {
        if (IsEditorInit()) {
            editor.layout();
        }
    }

    //
    // exports
    //

    window.SetMonacoCode = SetMonacoCode;
    window.RefreshEditorLayout = RefreshEditorLayout;

    //
    // finished loading client page
    //

    $(setUpUI);

})(window);
