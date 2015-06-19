//
// Copyright (C) Microsoft. All rights reserved.
//

if (!Instruction) {
  var Instruction = {};

  Instruction.CodeCol           = 2;
  Instruction.DestCol           = 4;
  Instruction.EqualsCol         = 20;
  Instruction.OpcodeCol         = 23;
  Instruction.Op1Col            = 38;
  Instruction.OperandSeparator  = 54;
  Instruction.Op2Col            = 56;

  Instruction.NegOne = 4294967295;  // -1 unsigned 32 bit

  Instruction.GetStatementIndex = function(inst) {
    var index = inst.statementIndex;
    if (inst.statementIndex === undefined) {
      index = Instruction.NegOne;
    }
    return index;
  };

  Instruction.BeginStatementBlock = function(inst, displayTags) {
    if (displayTags) {
      var index = Instruction.GetStatementIndex(inst);
      if (index === undefined) {
        Output.WriteZeroLength('<div class="ir-statement">');
      } else {
        Output.WriteZeroLength('<div class="ir-statement ir-statement-full" name="ir-statement-full-{0}">'.format(index));
      }
    }
  };

  Instruction.BeginStatement = function(inst, displayTags) {
    if (displayTags) {
      Output.BlankLine();
      Instruction.StatementToggleShow(inst, displayTags);
      Instruction.BeginStatementBlock(inst, displayTags);
    }
  };

  Instruction.EndStatement = function(inst, displayTags) {
    if (displayTags) {
      Output.WriteZeroLength('</div>');
    }
  };

  Instruction.StatementToggleHide = function(inst, displayTags) {
    if (displayTags) {
      var index = Instruction.GetStatementIndex(inst);
      Output.WriteZeroLength('<span class="ir-statement-toggle-hide invisible" name="ir-statement-hide-{0}">'.format(index));
      Output.Write('(-)');
      Output.WriteZeroLength('</span>');
    }
  };

  Instruction.StatementToggleShow = function(inst, displayTags) {
    if (displayTags) {
      var index = Instruction.GetStatementIndex(inst);
      var number = index;
      if (index == Instruction.NegOne) {
        number = -1;
      }

      Output.WriteZeroLength('<div class="ir-statement ir-statement-compact hidden" name="ir-statement-compact-{0}">'.format(index));
      Output.WriteZeroLength('<div class="ir-inst">');

      Output.WriteZeroLength('<span class="ir-statement-toggle-show" name="ir-statement-show-{0}">'.format(index));
      Output.Write('(+)');
      Output.WriteZeroLength('</span>');

      Output.WriteZeroLength('<span class="statement" name="statement-{0}">'.format(index));
      Output.SetColumn(Instruction.CodeCol);
      Output.Write('Line {1}, Col {2}: [Statement #{0}]  '.format(number, inst.line, inst.col));
      Output.WriteZeroLength('</span>');

      Output.WriteZeroLength('<span class="source" name="source-{0}">'.format(index));
      Output.Write(_.escape(inst.source));
      Output.WriteZeroLength('</span>');

      Output.WriteZeroLength('</div>');
      Output.WriteZeroLength('</div>');
    }
  };

  Instruction.BeginInstruction = function(inst, displayTags) {
    if (displayTags) {
      Output.WriteZeroLength('<div class="ir-inst">');
    }
  };

  Instruction.EndInstruction = function(inst, displayTags) {
    if (displayTags) {
      Output.WriteZeroLength('</div>');
    }
  };

  Instruction.Dump = function(inst, displayTags, metadata) {
    Output.SetHtmlSpacesStyle(displayTags);  // view HTML spaces and tags

    if (inst.opcode == "FunctionEntry") {
      Instruction.BeginStatementBlock(inst, displayTags);
    }

    if (inst.opcode == "StatementBoundary") {
      Instruction.BeginStatement(inst, displayTags);
    }

    Instruction.BeginInstruction(inst, displayTags);

    if (inst.label)
    {
      Output.NewLine();
      if (displayTags) {
        Output.WriteZeroLength('<span class="ir-label" name="label-{0}">'.format(inst.label));
      }
      Output.Write("L{0}".format(inst.label));
      if (displayTags) {
        Output.WriteZeroLength('</span>');
      }
      Output.Write(":");
    }
    else
    {
      if (inst.dst) {
        Output.SetColumn(Instruction.DestCol);
        new Operand(inst.dst, metadata).Dump(displayTags);

        Output.SetColumn(Instruction.EqualsCol);
        Output.Write("=");
      }

      if (inst.opcode == "StatementBoundary")
      {
        Output.Flush();
        Instruction.StatementToggleHide(inst, displayTags);

        //
        // print source line
        //

        if (!(inst.statementIndex === undefined)) {
          Output.SetColumn(Instruction.CodeCol);
          if (displayTags) {
            Output.WriteZeroLength('<span class="statement" name="statement-{0}">'.format(inst.statementIndex));
          }
          if (!(inst.line === undefined)) {
            Output.Write('Line {0}, '.format(inst.line));
          }
          Output.Write('Col {0}:  '.format(inst.col));
          if (displayTags) {
            Output.WriteZeroLength('</span>');
          }

          if (displayTags) {
            Output.WriteZeroLength('<span class="source" name="source-{0}">'.format(inst.statementIndex));
          }
          Output.Write(_.escape(inst.source));
          if (displayTags) {
            Output.WriteZeroLength('</span>');
          }

          Output.NewLine();
        }

        //
        // print StatementBoundary
        //

        if (displayTags) {
          Output.WriteZeroLength('<span class="statement" name="statement-{0}">'.format(inst.statementIndex));
        }

        Output.SetColumn(Instruction.OpcodeCol);
        Output.Write(inst.opcode);
        Output.SetColumn(Instruction.Op1Col);

        if (inst.statementIndex === undefined)
        {
          Output.Write("#-1");
        }
        else
        {
          Output.Write("#{0}".format(inst.statementIndex));
        }

        if (displayTags) {
          Output.WriteZeroLength('</span>');
        }
      }
      else
      {
        Output.SetColumn(Instruction.OpcodeCol);
        Output.Write(inst.opcode);
      }

      if (inst.src1) {
        Output.SetColumn(Instruction.Op1Col);
        new Operand(inst.src1, metadata).Dump(displayTags);
      }

      if (inst.src2) {
        Output.SetColumn(Instruction.OperandSeparator);
        Output.Write(",");

        Output.SetColumn(Instruction.Op2Col);
        new Operand(inst.src2, metadata).Dump(displayTags);
      }
    }

    if (inst.branch) {
      Output.SetColumn(Instruction.Op1Col);

      if (displayTags) {
        Output.WriteZeroLength('<span class="ir-label" name="label-{0}">'.format(inst.branch));
      }
      Output.Write("L{0}".format(inst.branch));
      if (displayTags) {
        Output.WriteZeroLength('</span>');
      }
    }

    Instruction.EndInstruction(inst, displayTags);

    if (!inst.next || inst.next.opcode == "StatementBoundary") {
      Instruction.EndStatement(inst, displayTags);
    }

    Output.Flush();
  };
}
