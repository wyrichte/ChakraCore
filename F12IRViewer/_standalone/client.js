;(function(window, undefined) {

    var ID = new Date().getTime();
    var BUILDS = {};
    var editor;

    function updateViewer(ir) {
        DisplayIRData(ir);
    };

    var doTimeout = false;
    function parseTimeoutCallback() {
        if (doTimeout) {
            doTimeout = false;
            $('#status').html('<span class="label label-warning">Timeout! (Check flags or shorten input function.)</span>');
        }
    }

    function getCurrentJSHOSTParams() {
        return {
            code: editor.getModel().getValue(),
            switches: $('#switches input').val().toString(),
            clientID: ID
        }
    }

    function parseJSCode() {
        $('#resultOutput').html('<p></p>');
        $('#resultError').html('<p></p>');
        $('#status').html('<span class="label">Running...</span>');

        doTimeout = true;
        window.setTimeout(parseTimeoutCallback, 3000);

        $.ajax({
            url: 'run',
            data: JSON.stringify(getCurrentJSHOSTParams()),
            type: 'POST'
        }).done(function(result) {
            var stringBuilder;
            if (result.Error) {
                stringBuilder = '<p>' + result.Error.toString() + '</p>';
                $('#resultError').html(stringBuilder);

                doTimeout = false;
                $('#status').html('<span class="label label-warning">Error!</span>');
            }

            stringBuilder = result.Output.toString();
            //stringBuilder = stringBuilder.replace(/\>/g, "&gt;"); // escape >
            //stringBuilder = stringBuilder.replace(/\</g, "&lt;"); // escape <

            stringBuilder = _.escape(stringBuilder);
            $('#resultOutput').html(stringBuilder);
            stringBuilder = _.unescape(stringBuilder);

            try {
                var ir = JSON.parse(stringBuilder);
                updateViewer(ir);

                doTimeout = false;
                $('#status').html('<span class="label label-success">Done!</span>');
            } catch (e) {
                doTimeout = false;
                $('#status').html('<span class="label label-warning">Error! (Check flags or shorten input function.)</span>');
            }
        }).error(function(err) {
            console.log(err);
            stringBuilder = '<p>' + err + '</p>'
            $('#resultError').html(stringBuilder);

            doTimeout = false;
            $('#status').html('<span class="label label-warning">Error!</span>');
        });
    }

    function MonacoSetup() {
        // You don't need to know that the HTML language mode is located at vs/languages/html/html,
        // you can use the modes registry and simply use a mime type

        // Here is the list of supported mime types:
        // "text/plain", "text/x-coffeescript", "text/coffeescript", "text/css", "text/x-cpp", "text/x-handlebars-template",
        // "text/html", "text/x-java-source", "text/javascript", "application/json", "text/x-web-markdown", "text/x-csharp",
        // "text/x-cshtml", "application/x-php", "text/x-vb", "text/xml", "application/xml", "application/xaml+xml", "text/x-bat"
        Monaco.Editor.getOrCreateMode('text/javascript').then(function() {
            editor = Monaco.Editor.create(document.getElementById("editor"), {
                value: "\
/** JS Function **/\r\n\
function foo(a,b) {\r\n\
    return a+b;\r\n\
}\r\n",
                mode: "text/javascript"
            });
        }).then(parseJSCode);
    }

    function setUpUI() {
        $('#run').click(function() {
            parseJSCode();
        });

        require.config({
            baseUrl: "monaco"
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
