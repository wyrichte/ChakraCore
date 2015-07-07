///
/// <reference path="../shout/util.js"/>
///

function getSubmitScriptPath(file) {
    var dir = Environment.variables("SDXROOT") + "\\inetcore\\jscript\\tools\\ChakraTool\\submit"; // WARNING: hardcoded
    return dir + "\\" + file;
}

//
// Run a job asynchronously. Check job completion repeatedly at checkInterval (ms), until checkComplete
// returns true to indicate job completion. More jobs can be linked by "then".
//
function runAsync(checkComplete, checkInterval) {
    var _jobs = [];
    var _onError = function (e) { alert("ERROR: " + JSON.stringify(e)); }; // default handler

    function push(checkComplete, checkInterval) {
        var wasEmpty = _jobs.length == 0;
        _jobs.push({ run: checkComplete, interval: (checkInterval || 50) });
        if (wasEmpty) {
            start();
        }
    }

    function start() {
        if (_jobs.length == 0) return;

        setTimeout(function () {
            try {
                if (!_jobs[0].run || _jobs[0].run()) {
                    _jobs.shift(); // remove the job
                }
            } catch (e) {
                var handler = _onError;
                if (!handler || !handler(e)) {
                    _jobs = []; // clear jobs, unless handler returns true to indicate continuing remaining jobs.
                }
            }

            start(); // schedule next job
        }, _jobs[0].interval);
    }

    push(checkComplete, checkInterval);
    return {
        // Link another job after this job is done.
        then: function (nextCheckComplete, nextInterval) {
            push(nextCheckComplete, nextInterval);
            return this;
        },
        // Provide an error handler to handle exception. Default will alert and quit all jobs.
        onError: function (handler) {
            _onError = handler;
            return this;
        }
    };
}

//
// Run a console cmd asynchronously. Callback "checkResult(result)" when cmd finishes.
//
function runCmdAsync(cmd, checkResult, checkInterval) {
    var tmpFile = TempFileHelper.createNewTempFile("", Environment.fso);
    var fullCmd = 'powershell.exe -ExecutionPolicy bypass "{0}" "\'{1}\'" "\'{2}\'"'.format(
        getSubmitScriptPath("run.ps1"), cmd, tmpFile); // Quote script parameters only
    Environment.shell.Run(fullCmd, 0, false);

    var doc = new ActiveXObject("Microsoft.XMLDOM");
    doc.async = false;

    return runAsync(function () {
        var result;
        try {
            if (doc.load(tmpFile)) {
                result = {};
                var root = doc.documentElement;
                for (var i = 0; i < root.childNodes.length; i++) {
                    var node = root.childNodes[i];
                    result[node.tagName] = node.text;
                }

                var err = result.ERROR || (result.state != "Completed" ? ("state: " + result.state + " " + result.stderr) : undefined);
                if (result.ERROR != err) {
                    result.ERROR = err;
                }
                result.output = function () { return this.stdout.replace(/\r/g, "").split("\n"); }
            }
        } catch (e) {
            // Eat exception while output not ready
            // alert(JSON.stringify(e));
        }

        if (result) {
            try {
                if (result.ERROR) {
                    throw new Error(result.ERROR);
                }
                checkResult(result);
            } finally {
                Environment.fso.DeleteFile(tmpFile);
            }
        }
        return !!result;
    }, checkInterval);
}

//
// CodeFlow helper
//
var CF = (function () {
    // Look at Name, Description lines for bug number patterns.
    function guessBugs(review) {
        var bugs = {};

        // Supported format:
        //var dat = [
        //    "12345 234567",         // Manual multiple bugs (CodeFlow WorkItems), no prefix
        //    "VSO:12345 VSO:23456",  // Manual multiple bugs (CodeFlow WorkItems)
        //    "VSO: 12345: some description",
        //    "VSO 12345: some description",
        //    "VSO: 12345 some description",
        //    "VSO 12345 some description",
        //    "DEVDIV2:12345 DEVDIV2:234567", // Ignore DEVDIV workitems, IECAT not hooked up
        //    "OS: Bug 620694: Assertion when evaluating 'new Map();' in F12" // Paste from Bugger
        //];
        function check(line) {
            var pat = /^\s*(MSFT|OS|OSG|TH|VSO)?(\s*:\s*(Bug)?)?\s*(\d\d\d\d\d+)(\s*:)?\s*(\S.*)?$/i;
            while (line) {
                var r = pat.exec(line);
                if (r) {
                    bugs[r[4]] = 1; // bug number
                    line = r[6]; // remaining
                } else {
                    return line;
                }
            }
        }

        check(review.WorkItems);
        review.Name = check(review.Name) || review.Name; // strip bug# from Title. IECAT will prefix it.
        review.Description.split("\n").forEach(function (line) { check(line); });
        review.bugs = Object.keys(bugs);
    }

    var CFCMD = "\\\\codeflow\\public\\cf.cmd ";
    return {
        // Query code flow dashboard and parse the output
        dashboard: function (onReviews) {
            return runCmdAsync(CFCMD + "dashboard", function (result) {
                var reviews = [];
                var review;

                var CREATEDBY = 0, DONE = 1;
                var state = -1;
                result.output().forEach(function (line) {
                    if (state < CREATEDBY) {
                        if (line.match(/^Reviews Created by/)) {
                            state = CREATEDBY;
                            debugOut("Created by");
                        }
                    } else if (state == CREATEDBY) {
                        debugOut("Line:" + JSON.stringify(line));
                        var r = /^\s*((\w+-)?\w+-\w+)\s+(\S.*)$/.exec(line);
                        if (r) {
                            debugOut("Review " + JSON.stringify(r));
                            if (review) {
                                reviews.push(review);
                            }
                            review = { id: r[1], title: r[3], signedoff: [] };
                        } else if (review) {
                            var s = /^\s*\(.\)\s*(\w+)\s*:\s*(\w+)\s*$/.exec(line);
                            if (s && s[2] == "SignedOff") {
                                review.signedoff.push(s[1]);
                            }
                        }

                        if (/^\w/.exec(line)) {
                            debugOut("Done");
                            state = DONE;
                        }
                    }
                });

                if (review) { // push last review
                    reviews.push(review);
                }
                onReviews(reviews);
            });
        },

        // Query code flow review info
        info: function (id, onInfo, onError) {
            return runCmdAsync(CFCMD + "info " + id, function (result) {
                var review = { id: id, signedoff: [] };

                var fields = ["Name", "WorkItems", "LatestCodePackage"];
                var FIELD = 0, REVIEWER = 1, DESCRIPTION = 2;
                var state = FIELD;
                result.output().forEach(function (line) {
                    if (state == REVIEWER) {
                        var r = /^\s*(\w+)\s+\w+\s+(\w+)\s*$/.exec(line);
                        if (r) {
                            if (r[2] == "SignedOff") {
                                review.signedoff.push(r[1]);
                            }
                        } else if (line.match(/:/)) {
                            state = FIELD; // Change state to FIELD
                        }
                    }

                    if (state == FIELD) {
                        var r = /^\s*(\w+)\s*:\s*(\S(.*\S)?)\s*$/.exec(line);
                        if (r) {
                            if (r[1] == "ReviewerStatus") {
                                state = REVIEWER;
                            } else if (r[1] == "Description") {
                                state = DESCRIPTION;
                                review.Description = line.replace(/^\s*\w+\s*:\s*(\S.*)$/, "$1"); // includes trailing spaces
                            } else {
                                fields.some(function (field) {
                                    if (field == r[1]) {
                                        review[field] = r[2];
                                        return true;
                                    }
                                });
                            }
                        }
                    } else if (state == DESCRIPTION) {
                        review.Description += "\n" + line;
                    }
                });
                review.Description = review.Description || ""; //TEMP: CodeFlow issue, missing Description

                guessBugs(review);
                onInfo(review);
            });
        },

        complete: function (id) {
            return runCmdAsync('{0} complete {1} -message "Thanks!"'.format(CFCMD, id), function (result) {
                // ignore result
            });
        }
    };
})();

//
// Create an IE automation object.
//
function make_ie(visible) {
    var ie = new ActiveXObject("InternetExplorer.Application");
    var jobs = runAsync();

    return {
        navigate: function (url, next) {
            jobs.then(function () {
                ie.navigate(url);
                if (visible) {
                    ie.visible = !!visible;
                }
                return true;
            });

            return this.then(next);
        },
        then: function (next, checkInterval) {
            if (next) {
                jobs.then(function () {
                    if (!ie.busy) {
                        return next();
                    }
                }, checkInterval || 100);
            }
            return this;
        },
        onError: function (handler) {
            jobs.onError(handler);
            return this;
        },
        busy: function () {
            return ie.busy;
        },
        getElementById: function (id) {
            return ie.document.getElementById(id);
        },
        close: function () {
            ie.quit();
            ie = null;
        },

        get visible() { return ie.visible; },
        set visible(b) { ie.visible = b; }
    };
}

//
// IECAT helper
//
var IECAT = (function () {
    var ie, queue, user, oldJobs;

    function init() {
        queue = "\\\\iesnap\\queue\\{0}".format(Environment.variables("_BuildBranch"));
        user = Environment.variables("_NTUSER");
        ie = make_ie();
    }

    function getJobs() {
        var jobs = {};
        var root = Environment.fso.getFolder(queue);
        var dirs = new Enumerator(root.subFolders);
        for (dirs.moveFirst() ; !dirs.atEnd() ; dirs.moveNext()) {
            jobs[dirs.item()] = 1;
        }
        return jobs;
    }

    // Call this before submit to take a snapshot of snap jobs
    function snapshotJobs() {
        oldJobs = getJobs();
    }

    function updateSnapJobXml(job, onState) {
        var path;
        return runAsync(function () {
            var newJobs = Object.keys(getJobs())
                .filter(function (dir) { return !oldJobs.hasOwnProperty(dir); })
                .filter(function (dir) { return Environment.fso.fileExists("{0}\\{1}.xml".format(dir, user)); });
            if (newJobs.length == 0) {
                return false;
            } else if (newJobs.length > 1) {
                throw new Error("Found multiple new jobs of yours. Will not tweak job xml.");
            }

            path = "{0}\\{1}.xml".format(newJobs[0], user);
            return true;
        }, 500).then(function () {
            onState("Updating {0}...".format(path));
            var fullCmd = 'powershell.exe -ExecutionPolicy bypass "{0}" "\'{1}\'" "\'{2}\'"'.format(
                getSubmitScriptPath("chjob.ps1"), path, job.reviewer); // Quote script parameters only
            Environment.shell.Run(fullCmd, 0, /*waitOnReturn*/true);
            return true;
        });
    }

    return {
        // submit a job to IECAT through IE automation
        submit: function (job, onState) {
            init();
            var hasBugs = job.bugs.length > 0;
            var newCatUrl = hasBugs ?
                "http://iecat/createchange.aspx?typeid=113"/*snap only*/ :
                "http://iecat/createchange.aspx?typeid=120"/*future work*/;

            onState('Visiting IECAT...');
            ie.navigate(newCatUrl, function () {
                if (hasBugs) {
                    onState('Adding bugs... (slow, please wait)');
                    ie.getElementById("ctl00_PageContents_BugDatabaseDropDownList").value = 24; //OSG VSO
                    ie.getElementById("ctl00_PageContents_AddBugTextBox").value = job.bugs;
                    ie.getElementById("ctl00_PageContents_AddBugButton").click();
                } else {
                    onState('Skipped bugs for Future Work checkin.');
                }
                return true;
            }).then(function () {
                if (hasBugs) {
                    if (ie.getElementById("ctl00_PageContents_AddBugTextBoxCustomValidator").style.display != "none") {
                        throw new Error("ERROR: Unrecognized bugs: {0}".format(job.bugs));
                    }
                    if (ie.getElementById("ctl00_PageContents_AddedBugsListBox").children.length) {
                        return true; // Bugs added successfully
                    }
                    return false; // wait
                }
                return true;
            }).then(function () {
                var fullCmd = 'powershell.exe -ExecutionPolicy bypass "{0}" "\'{1}\'"'.format(
                    getSubmitScriptPath("fillupload.ps1"), job.pack); // Quote script parameters only
                Environment.shell.Run(fullCmd, 0, false);

                document.parentWindow.clipboardData.setData("text", job.pack); // Also copy to clipboard
                onState('Uploading code pack...');
                ie.getElementById("ctl00_PageContents_AddCodePackageFileUpload").click();
                return true;
            }).then(function () {
                ie.getElementById("ctl00_PageContents_NextButton").click();
                return true;
            }).then(function () {
                ie.getElementById("ctl00_PageContents_TitleTextBox").value = job.title.replace(/:(\s*\d+)/g, '_$1').substr(0, 130); // IECAT rejects long title
                ie.getElementById("ctl00_PageContents_FixDescriptionTextBox").value = job.description.replace(/:(\s*\d+)/g, '_$1'); // IECAT doesn't like BUG:1234 in description
                ie.getElementById("ctl00_PageContents_NextButton").click();
                return true;
            }).then(function () {
                onState('Submitting to IECAT...');
                snapshotJobs(); // Snapshot existing jobs before submitting, to help determine the new job
                ie.getElementById("ctl00_PageContents_FinishButton").click();
                return true;
            }).then(function () {
                onState('Updating snap...');
                updateSnapJobXml(job, onState).then(function () {
                    ie.close(); // close IE now
                    ie = undefined;
                    onState('Completing CodeFlow review...');
                    CF.complete(job.id).then(function () {
                        onState('Done');
                        return true;
                    });
                    return true;
                }).onError(function (e) {
                    onState(JSON.stringify(e), "error");
                    ie.visible = true;
                });
                return true;
            }).onError(function (e) {
                onState(JSON.stringify(e), "error");
                ie.visible = true;
            });
        },

        // Make IE visible to resolve problems manually
        show: function () {
            if (ie) {
                ie.visible = true;
            }
        }
    };
})();
