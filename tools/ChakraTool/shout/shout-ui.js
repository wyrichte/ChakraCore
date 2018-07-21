///
/// Shout-UI.js
/// 
/// Rendering layer for Shout- has various templates interspersed through
/// the code
///

var View = new function() {
     var oThis = this;

     // Interesting DOM elements
     var messageDiv = null;
     var resultDiv = null;
     var warningsDiv = null;

     var globalInterestingLines = {};
     var globalInterestingLinesCategories = { assert: 0, crash: 0, error: 0 };
     var assertionRegex = /^ASSERTION \d+\:/;
     var fatalErrorRegex = /^FATAL ERROR\:/;
     var errorRegex = /^ERROR:/;

     function getInterestingLines(output) {
         var lines = output.split('\n');
         var interestingLines = [];
         for (var i = 0; i < lines.length && interestingLines.length < 10; i++) {
             var line = lines[i];
             var interestingReason = null;

             if (assertionRegex.test(line))
             {
                 interestingReason = 'assert';
             }
             else if (fatalErrorRegex.test(line)) 
             {
                 interestingReason = 'crash';
             }
             else if (errorRegex.test(line))
             {
                 interestingReason = 'error';
             }

             if (interestingReason != null) { 
                 globalInterestingLinesCategories[interestingReason]++;

                 interestingLines.push(line); 
                 if (!globalInterestingLines[line]) {
                     globalInterestingLines[line] = 1;
                 } else {
                     globalInterestingLines[line]++;
                 }
             }
         }

         return interestingLines;
     }

     function getInterestingLinesAnalysis() {
         var html = [];
         html.push('<div class="container">');

         html.push('<div class="row">');
         html.push('<ul class="list-group col-lg-2">');
         html.push('<li class="list-group-item">');
         html.push('<span class="badge">{0}</span>Assertions'.format(globalInterestingLinesCategories['assert']));
         html.push('</li>');
         html.push('<li class="list-group-item">');
         html.push('<span class="badge">{0}</span>Crashes'.format(globalInterestingLinesCategories['crash']));
         html.push('</li>');
         html.push('<li class="list-group-item">');
         html.push('<span class="badge">{0}</span>Other errors'.format(globalInterestingLinesCategories['error']));
         html.push('</li>');

         html.push('</ul>');
         html.push('</div>');
         html.push('</div>');
         return html.join('');
     }

     function getInterestingLinesSummary() {
         var counts = [];
         for (var line in globalInterestingLines) {
             counts.push(globalInterestingLines[line]);
         }

         counts.sort();
         var threshold = (counts.length <= 10 ? counts[counts.length - 1] : counts[10]);

         var summaryLines = [];
         for (var line in globalInterestingLines) {
             if (globalInterestingLines[line] > threshold) {
                 summaryLines.push({ line: line, freq: globalInterestingLines[line]});
             }
         }

         summaryLines.sort(function(a, b) { return a.val - b.val; });

         var html = [];
         
         html.push('<div class="container">');
         for (var i = 0; i < Math.min(10, summaryLines.length); i++) {
             html.push('<div class="row">');
             html.push('<h5>Frequency: {0}</h5>'.format(summaryLines[i].freq));
             html.push('<pre>{0}</pre>'.format(summaryLines[i].line));
             html.push('</div>');
         }

         html.push('</div>');
         
         return html.join('');
     }

     function renderPanel(heading, content, id) {
         var html = [];

         html.push('<div class="panel panel-default" id="{0}">'.format(id));
         html.push('<div class="panel-heading"><h3 class="panel-title">{0}</h3></div>'.format(heading));
         html.push('<div class="panel-body">{0}</div>'.format(content));
         html.push('</div>');

         return html.join('');
     }

     function renderSummary(context) {
         var html = [];

         var contents = [];

         contents.push('<dl class="dl-horizontal" style="width: 300px">');

         context.forEachTest(
             function(test, failures, context) {
                 contents.push('<dt>' + test + "</dt>");
                 contents.push('<dd>' + failures.count + " failures</dd>");              
             });

         contents.push('</dl>');

         html.push(renderPanel('Results of test run', contents.join(''), 'resultsPanel'));

         if (context.totalFailures > 0) {
             html.push(renderPanel('Failure analysis', getInterestingLinesAnalysis(), 'analysisPanel'));
             html.push(renderPanel('Most frequent errors', getInterestingLinesSummary(), 'frequencyPanel'));
         }

         return html.join('');
     }

     function renderFailuresInGroup(testName, groupName, group) {
         var html = [];
         
         var copyButtonTemplate = '<button class="btn btn-default" onclick="copyTestCommandline({0})">Copy command line</button>';
         var copyDebugButtonTemplate = '<button class="btn btn-default" onclick="copyDebugTestCommandline({0})">Copy debug command line</button>';
         var displayButtonTemplate = '<button class="btn btn-default" onclick="displayOutputForTest({0})">Display output</button>';
         var diffButtonTemplate = '<button class="btn btn-default" onclick="showDiff({0})">Diff with baseline</button>';
         var buttonGroupTemplate = '<div class="btn-group">{0}</div>';

         html.push('<ul class="list-group">');
         for (var i = 0; i < group.length; i++) {
             var test = group[i];
             var testId = "'{0}', '{1}', {2}".format(testName, groupName, i);
             
             html.push('<li class="list-group-item">');
             html.push('<div class="container">');
             html.push('<div class="row">');
             html.push('<div class="col-lg-4">');
             html.push('<span><strong>' + test.test + "</strong></span>");
             html.push('</div>');

             var buttons = [];
             if (test.baseline != "<unknown>") {
                 buttons.push(diffButtonTemplate.format(testId));
             }
             
             buttons.push(copyButtonTemplate.format(testId));
             buttons.push(copyDebugButtonTemplate.format(testId));
             buttons.push(displayButtonTemplate.format(testId));
             
             html.push('<div class="pull-right" style="margin-right: 20px;">');
             html.push(buttonGroupTemplate.format(buttons.join('')));
             html.push('</div>');
             html.push('</div>');

             html.push('<div class="row">');
             html.push('<h5>Command line:</h5>');
             html.push('<pre>');
             html.push(test.commandLine);
             html.push('</pre>');
             html.push('</div>');

             var interestingLines = getInterestingLines(test.output);
             
             html.push('<div class="row">');
             if (interestingLines.length > 0) {
                 html.push('<h5>Interesting lines:</h5>');
                 html.push('<pre class="interestingLines">');
                 html.push(interestingLines.join('\n'));
                 html.push('</pre>');
             } else {
                 html.push('<h5 class="alert alert-warning">No asserts or errors found- please diff with baseline</h5>');
             }

             html.push('</div>');
             html.push('</div>');

             html.push('</li>');
         }

         html.push('</ul>');

         return html.join('');
     }

     var expandedStates = {};
     function expandTest(el, testName) {
         var tags = document.getElementsByClassName('accordion-body');
         
         var expanded = expandedStates[testName];
         for (var i = 0; i < tags.length; i++) {
             if (tags[i].id.indexOf(testName + "Accordion") == 0) {
                 var classes = tags[i].className.split(/\s+/);
                 for (var j = 0; j < classes.length; j++) {
                     if (!expanded && classes[j] == 'in')
                         continue;
                     else if (expanded && classes[j] == 'in')
                     classes[j] = '';
                 }

                 if (!expanded) {
                     tags[i].className += " in";
                 } else {
                     tags[i].className = classes.join(' ');
                 }
             }
         }

         expanded = !expanded;
         el.innerText = (expanded ? "Collapse all" : "Expand all");
     }

     function renderFailureGroup(testName, groupName, group, accordionParent) {
         var html = [];

         var headingTemplate = '<a class="accordion-toggle" data-toggle="collapse" data-parent="#{2}" href="#{3}">{0} ({1} tests)</a>';
         var accordionBodyId = accordionParent + groupName + "Group";

         var accordionBodyTemplate = '<div id="{0}" class="accordion-body collapse in"><div class="accordion-inner">{1}</div></div>';
         html.push('<div class="accordion-group">');
         html.push('<div class="accordion-heading">');
         html.push(headingTemplate.format(groupName, group.length, accordionParent, accordionBodyId));
         html.push(accordionBodyTemplate.format(accordionBodyId, renderFailuresInGroup(testName, groupName, group)));
         html.push('</div>');
         html.push('</div>');
         expandedStates[testName] = true;
         return html.join('');
     }

     function renderFailuresForTest(test, failures, context) {
         var accordionRootName = test + 'Accordion';

         var html = [];

         html.push('<h2>' + test + '</h2>');
         html.push('<h3>' + failures.count + ' failure' +
                   (failures.count != 1 ? 's' : '')); 
         html.push('<button class="btn btn-link btn-lg no-ol" onclick="expandTest(this, \'{0}\')">Collapse all</button>'.format(test));
         html.push('</h3>');

         html.push('<div class="accordion" id="' + accordionRootName + '">');
         for (var i = 0; i < failures.groups.length; i++) {
             var group = failures.groups[i];
             html.push(renderFailureGroup(test, group, failures[group], accordionRootName));
         }
         html.push('</div>');

         return html.join('');
     }

     window.displayOutputForTest = function(testName, groupName, id) {
         var test = Environment.failedTests[testName];
         var group = test[groupName];
         var failure = group[id];

         var child = window.open("about:blank", "_blank");
         var output = "Baseline: {0}\nOutput:\n\n{1}".format(failure.baseline, failure.output);
         child.document.body.innerText = output;
     }

     window.displayError = function(e) {
         html = "<h2 class='error'>" + e + "</h2>";
         displayMessage(html);
     }

     window.displayMessage = function(s)
     {
         messageDiv.innerHTML = (s + "<br />");
     }

     oThis.init = function()
     {
         messageDiv   = document.getElementById("msgDiv");
         resultDiv    = document.getElementById("resultDiv");
         warningsDiv  = document.getElementById("warningsDiv");
     }

     oThis.render = function(ctx)
     {
         if (Environment.missingFiles.length > 0) {
             var html = [];

             html.push('<div class="alert alert-warning">');
             html.push('<strong>Warning: </strong>');
             html.push('No log files found in following folders: ');
             html.push('<p>' + Environment.missingFiles.join(', ') + "</p>");
             html.push('</div>');

             warningsDiv.innerHTML = html.join('');
             warningsDiv.className = '';
         }

         if (!!Environment.inProgressWarning) {
             var html = [];

             html.push('<div class="alert alert-warning">');
             html.push('<strong>Warning: </strong>');
             html.push('Tests for <b>' + Environment.inProgressWarning +
             "</b> are currently in progress. Results may be incomplete");
             html.push('</div>');

             warningsDiv.innerHTML += html.join('');
             warningsDiv.className = '';
         }


         var lines = [];
         var context = Environment.failedTests;

         var tabs = [];

         var listItemTemplate = '<li {2}><a href="#{1}" data-toggle="tab">{0}</a></li>';
         var tabContentTemplate = '<div class="tab-pane {1}" id="{0}">{2}</div>'
         tabs.push('<ul class="nav nav-tabs">');
         tabs.push(listItemTemplate.format('Summary', 'summary', 'class="active"'));
         context.forEachTest(function(testName, test, context) {
             tabs.push(listItemTemplate.format(testName, testName, ''));
         });

         tabs.push('</ul>');

         var content = [];
         content.push('<div class="tab-content">');
         
         context.forEachTest(
             function(testName, failures, context) {
                 content.push(tabContentTemplate.format(testName, '',
                                                        renderFailuresForTest(testName, failures, context)));            
             });

         
         // call render failures first so it does all the preprocessing of
         // failures already so that summary has interesting content to display
         content.splice(1, 0, tabContentTemplate.format('summary', 'active', renderSummary(context)));
         content.push('</div>');
         html = tabs.join('');
         html += content.join('');
         
         messageDiv.className = "invisible";
         resultDiv.innerHTML = html;
         resultDiv.className = "";

         var rootMarkup = document.getElementById('root').getElementsByTagName('div')[0].outerHTML;
         debugOut(rootMarkup);
         debugFlush();
     }

};