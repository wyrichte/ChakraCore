/// <reference path="../UnitTestFramework/UnitTestFramework.js" />
if (this.WScript && this.WScript.LoadScriptFile) { // Check for running in jc/jshost
    this.WScript.LoadScriptFile("..\\..\\core\\test\\UnitTestFramework\\UnitTestFramework.js");
    this.WScript.LoadScriptFile("enclib.js");
}

var default_options = {
    match: true, // dump tree match
    edits: true, // dump edit script
    full: true, // do full ast diff in this test
};

function verifyAstDiff(before, after, expected, options) {

    // Get JSON stringify representation to compare. This honors order. Ignore \n \r.
    function getString(obj) {
        return JSON.stringify(obj, null, 2).replace(/(\\r)|(\\n)/g, "");
    }

    var diff = enc.astDiff(before, after, options || default_options);
    assert.areEqual(getString(expected), getString(diff));
}

var tests = [
    {
        name: "string LCS",
        body: function () {
            [
                ["", "", ""],
                ["", "B", "+B"],
                ["A", "A", "A"],
                ["A", "AB", "A +B"],
                ["A", "ABC", "A +B +C"],
                ["A", "BA", "+B A"],
                ["A", "BCA", "+B +C A"],
                ["A", "BABC", "+B A +B +C"],
                ["A", "", "-A"],
                ["A", "B", "-A +B"],
                ["ABCE", "ABDE", "A B -C +D E"],
                ["ABCD", "ABCABX", "A B C -D +A +B +X"],
            ].forEach(function (t) {
                var lcs = EditTest.LCS(t[0], t[1]);
                assert.areEqual(t[2], lcs, "Test: " + t[0] + " ==> " + t[1]);
            });
        }
    },

    {
        name: "AST diff tests",
        body: function () {
            verifyAstDiff("", "1", {
                "Match": [
                  "undefined@undefined ",
                  "undefined@undefined "
                ],
                "Edits": [
                  "Insert  NumberLit@0 1"
                ]
            });

            verifyAstDiff("1", "", {
                "Match": [
                  "undefined@undefined ",
                  "undefined@undefined "
                ],
                "Edits": [
                  "Delete  NumberLit@0 1"
                ]
            });

            verifyAstDiff("1", "1 + 2", {
                "Match": [
                  [
                    "Unit@0 1",
                    "Unit@0 1 + 2"
                  ],
                  [
                    "NumberLit@0 1",
                    "NumberLit@0 1"
                  ],
                  [
                    "@0 ",
                    "@0 "
                  ]
                ],
                "Edits": [
                  "Insert  AddOper@0 1 + 2",
                  "Move    NumberLit@0 1 -> NumberLit@0 1",
                  "Insert  NumberLit@4 2"
                ]
            });

            verifyAstDiff("1 + 2", "1 + 3", {
                "Match": [
                  [
                    "Unit@0 1 + 2",
                    "Unit@0 1 + 3"
                  ],
                  [
                    "AddOper@0 1 + 2",
                    "AddOper@0 1 + 3"
                  ],
                  [
                    "NumberLit@0 1",
                    "NumberLit@0 1"
                  ],
                  [
                    "NumberLit@4 2",
                    "NumberLit@4 3"
                  ],
                  [
                    "@0 ",
                    "@0 "
                  ]
                ],
                "Edits": [
                  "Update  NumberLit@4 2 -> NumberLit@4 3"
                ]
            });

            verifyAstDiff("1 + 2 + 3 + 4 + 5", "1 + 3 + 5", {
                "Match": [
                  [
                    "Unit@0 1 + 2 + 3 + 4 + ",
                    "Unit@0 1 + 3 + 5"
                  ],
                  [
                    "AddOper@0 1 + 2 + 3 + 4 + ",
                    "AddOper@0 1 + 3 + 5"
                  ],
                  [
                    "AddOper@0 1 + 2 + 3 + 4",
                    "AddOper@0 1 + 3"
                  ],
                  [
                    "NumberLit@0 1",
                    "NumberLit@0 1"
                  ],
                  [
                    "NumberLit@8 3",
                    "NumberLit@4 3"
                  ],
                  [
                    "NumberLit@16 5",
                    "NumberLit@8 5"
                  ],
                  [
                    "@0 ",
                    "@0 "
                  ]
                ],
                "Edits": [
                  "Move    NumberLit@0 1 -> NumberLit@0 1",
                  "Move    NumberLit@8 3 -> NumberLit@4 3",
                  "Delete  AddOper@0 1 + 2 + 3",
                  "Delete  AddOper@0 1 + 2",
                  "Delete  NumberLit@4 2",
                  "Delete  NumberLit@12 4"
                ]
            });

            verifyAstDiff("1 + 2 + 3 + 4 + 5", "1 + 3 + 6", {
                "Match": [
                  [
                    "Unit@0 1 + 2 + 3 + 4 + ",
                    "Unit@0 1 + 3 + 6"
                  ],
                  [
                    "AddOper@0 1 + 2 + 3 + 4",
                    "AddOper@0 1 + 3 + 6"
                  ],
                  [
                    "AddOper@0 1 + 2 + 3",
                    "AddOper@0 1 + 3"
                  ],
                  [
                    "NumberLit@0 1",
                    "NumberLit@0 1"
                  ],
                  [
                    "NumberLit@4 2",
                    "NumberLit@8 6"
                  ],
                  [
                    "NumberLit@8 3",
                    "NumberLit@4 3"
                  ],
                  [
                    "@0 ",
                    "@0 "
                  ]
                ],
                "Edits": [
                  "Move    AddOper@0 1 + 2 + 3 + 4 -> AddOper@0 1 + 3 + 6",
                  "Update  NumberLit@4 2 -> NumberLit@8 6",
                  "Move    NumberLit@4 2 -> NumberLit@8 6",
                  "Move    NumberLit@0 1 -> NumberLit@0 1",
                  "Delete  AddOper@0 1 + 2 + 3 + 4 + ",
                  "Delete  AddOper@0 1 + 2",
                  "Delete  NumberLit@12 4",
                  "Delete  NumberLit@16 5"
                ]
            });

            verifyAstDiff("1", "var b", {
                "Match": [
                  "undefined@undefined ",
                  "undefined@undefined "
                ],
                "Edits": [
                  "Insert  VarDecl@0 var b",
                  "Delete  NumberLit@0 1"
                ]
            });

            verifyAstDiff("var a", "var b", {
                "Match": [
                  [
                    "Unit@0 var a",
                    "Unit@0 var b"
                  ],
                  [
                    "VarDecl@0 var a",
                    "VarDecl@0 var b"
                  ],
                  [
                    "@0 ",
                    "@0 "
                  ]
                ],
                "Edits": [
                  "Update  VarDecl@0 var a -> VarDecl@0 var b"
                ]
            });

            verifyAstDiff("var a = 1", "var b = 1", {
                "Match": [
                  [
                    "Unit@0 var a = 1",
                    "Unit@0 var b = 1"
                  ],
                  [
                    "VarDecl@0 var a = 1",
                    "VarDecl@0 var b = 1"
                  ],
                  [
                    "NumberLit@8 1",
                    "NumberLit@8 1"
                  ],
                  [
                    "@0 ",
                    "@0 "
                  ]
                ],
                "Edits": [
                  "Update  VarDecl@0 var a = 1 -> VarDecl@0 var b = 1"
                ]
            });

            verifyAstDiff("var a = 1", "var b = 2", {
                "Match": [
                  [
                    "Unit@0 var a = 1",
                    "Unit@0 var b = 2"
                  ],
                  [
                    "VarDecl@0 var a = 1",
                    "VarDecl@0 var b = 2"
                  ],
                  [
                    "NumberLit@8 1",
                    "NumberLit@8 2"
                  ],
                  [
                    "@0 ",
                    "@0 "
                  ]
                ],
                "Edits": [
                  "Update  VarDecl@0 var a = 1 -> VarDecl@0 var b = 2",
                  "Update  NumberLit@8 1 -> NumberLit@8 2"
                ]
            });

            verifyAstDiff("var a = 1", "var a = 1; var b = 2", {
                "Match": [
                  [
                    "Unit@0 var a = 1",
                    "Unit@0 var a = 1; var b"
                  ],
                  [
                    "VarDecl@0 var a = 1",
                    "VarDecl@0 var a = 1"
                  ],
                  [
                    "NumberLit@8 1",
                    "NumberLit@8 1"
                  ],
                  [
                    "@0 ",
                    "@0 "
                  ]
                ],
                "Edits": [
                  "Insert  VarDecl@11 var b = 2",
                  "Insert  NumberLit@19 2"
                ]
            });

            verifyAstDiff(
                function foo() {
                    var a = 1;
                    var b = 2;
                },
                function foo() {
                    var b = 2;
                    var a = 1;
                },
                {
                    "Match": [
                      [
                        "Unit@0 function foo() {",
                        "Unit@0 function foo() {"
                      ],
                      [
                        "FuncDecl@0 function foo() {",
                        "FuncDecl@0 function foo() {"
                      ],
                      [
                        "@38 var a = 1;    ",
                        "@38 var b = 2;    "
                      ],
                      [
                        "VarDecl@38 var a = 1",
                        "VarDecl@70 var a = 1"
                      ],
                      [
                        "NumberLit@46 1",
                        "NumberLit@78 1"
                      ],
                      [
                        "VarDecl@70 var b = 2",
                        "VarDecl@38 var b = 2"
                      ],
                      [
                        "NumberLit@78 2",
                        "NumberLit@46 2"
                      ],
                      [
                        "@98 }",
                        "@98 }"
                      ],
                      [
                        "@0 ",
                        "@0 "
                      ]
                    ],
                    "Edits": [
                      "Update  FuncDecl@0 function foo() { -> FuncDecl@0 function foo() {",
                      "Reorder VarDecl@38 var a = 1 -> VarDecl@70 var a = 1"
                    ]
                });

            verifyAstDiff(
                function foo() {
                    var a = 1;
                    var b = 2;
                },
                function foo() {
                    var a = 1; //ignored comment
                    var x = 0.5;
                    var b = 2;
                },
                {
                    "Match": [
                      [
                        "Unit@0 function foo() {",
                        "Unit@0 function foo() {"
                      ],
                      [
                        "FuncDecl@0 function foo() {",
                        "FuncDecl@0 function foo() {"
                      ],
                      [
                        "@38 var a = 1;    ",
                        "@38 var a = 1; //ign"
                      ],
                      [
                        "VarDecl@38 var a = 1",
                        "VarDecl@38 var a = 1"
                      ],
                      [
                        "NumberLit@46 1",
                        "NumberLit@46 1"
                      ],
                      [
                        "VarDecl@70 var b = 2",
                        "VarDecl@122 var b = 2"
                      ],
                      [
                        "NumberLit@78 2",
                        "NumberLit@130 2"
                      ],
                      [
                        "@98 }",
                        "@150 }"
                      ],
                      [
                        "@0 ",
                        "@0 "
                      ]
                    ],
                    "Edits": [
                      "Update  FuncDecl@0 function foo() { -> FuncDecl@0 function foo() {",
                      "Insert  VarDecl@88 var x = 0.5",
                      "Insert  NumberLit@96 0.5"
                    ]
                });

            verifyAstDiff("a = function(){}", "a = function(){}", {
                "Match": [
                  [
                    "Unit@0 a = function(){}",
                    "Unit@0 a = function(){}"
                  ],
                  [
                    "AssignmentOper@0 a = function(){}",
                    "AssignmentOper@0 a = function(){}"
                  ],
                  [
                    "NameExpr@0 a",
                    "NameExpr@0 a"
                  ],
                  [
                    "FuncDecl@4 function(){}",
                    "FuncDecl@4 function(){}"
                  ],
                  [
                    "@15 }",
                    "@15 }"
                  ],
                  [
                    "@0 ",
                    "@0 "
                  ]
                ],
                "Edits": []
            });

            verifyAstDiff("a = function(){}", "a = function(){return b;}", {
                "Match": [
                  [
                    "Unit@0 a = function(){}",
                    "Unit@0 a = function(){r"
                  ],
                  [
                    "AssignmentOper@0 a = function(){}",
                    "AssignmentOper@0 a = function(){r"
                  ],
                  [
                    "NameExpr@0 a",
                    "NameExpr@0 a"
                  ],
                  [
                    "FuncDecl@4 function(){}",
                    "FuncDecl@4 function(){retur"
                  ],
                  [
                    "@15 }",
                    "@24 }"
                  ],
                  [
                    "@0 ",
                    "@0 "
                  ]
                ],
                "Edits": [
                  "Update  FuncDecl@4 function(){} -> FuncDecl@4 function(){retur",
                  "Insert  @15 return b;}",
                  "Insert  ReturnStmt@15 return b",
                  "Move    @15 } -> @24 }",
                  "Insert  NameExpr@22 b"
                ]
            });

            verifyAstDiff("for(;;){}", "for(;;i++){}", {
                "Match": [
                  [
                    "Unit@0 for(;;){}",
                    "Unit@0 for(;;i++){}"
                  ],
                  [
                    "ForStmtm@0 for(;;)",
                    "ForStmtm@0 for(;;i++)"
                  ],
                  [
                    "Block@7 {}",
                    "Block@10 {}"
                  ],
                  [
                    "@0 ",
                    "@0 "
                  ]
                ],
                "Edits": [
                  "Insert  PostIncExpr@6 i++",
                  "Insert  NameExpr@6 i"
                ]
            });
        }
    }
];

testRunner.run(tests, { verbose: WScript.Arguments[0] != "summary" });
