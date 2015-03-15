//******************************************************************************
//
// This file is part of the OpenHoldem project
//   Download page:         http://code.google.com/p/openholdembot/
//   Forums:                http://www.maxinmontreal.com/forums/index.php
//   Licensed under GPL v3: http://www.gnu.org/licenses/gpl.html
//
//******************************************************************************
//
// Purpose:
//
//******************************************************************************

#ifndef INC_CFORMULAPARSER_H
#define INC_CFORMULAPARSER_H

#define _AFXDLL 

#include "afx.h"
#include "atlstr.h"
#include "CFormulaFileSplitter.h"
#include "COHScriptList.h"
#include "CParseTreeNode.h"
#include "CParseTreeRotator.h"
#include "CTokenizer.h"

class CParserSymbolTable;

class CFormulaParser {
 public:
  CFormulaParser();
  ~CFormulaParser();
 public:
  void InitNewParse();
  void FinishParse();
 public:
  // This function will
  // * load the OpenPPL-library, if needed and posisble
  // * then load the formula file with user-defined bot-logic
  void ParseFormulaFileWithUserDefinedBotLogic(CArchive & formula_file);
 public:
  void ParseSingleFormula(CString name, CString function_text);
  void ParseSingleFormula(CString function_text);
  void ParseOpenPPLLibraryIfNeeded();
 public:
  static CString CurrentFunctionName();
  bool IsParsing()	                          { return _is_parsing; }
  CParserSymbolTable* p_parser_symbol_table() { return _p_parser_symbol_table; }
 private:
  void ParseFile(CArchive & formula_file);
 private:
  bool VerifyFunctionHeader(CString function_header);
  void ExpectMatchingBracketClose(int opening_bracket);
  bool ExpectConditionalThen();
  bool ExpectKeywordForce(int last_important_roken_ID);
  void CheckForExtraTokensAfterEndOfFunction();
 private:
  // OH-script and OpenPPL
  void ParseListBody(COHScriptList *list);
  TPParseTreeNode ParseFunctionBody();
  void ParseDebugTab(CString function_text);
 private:
  // OH-script
  TPParseTreeNode ParseExpression();
  TPParseTreeNode ParseBracketExpression();
  TPParseTreeNode ParseUnaryExpression();
  TPParseTreeNode ParseSimpleExpression();
  void ParseConditionalPartialThenElseExpressions(
	  TPParseTreeNode *then_expression, TPParseTreeNode *else_expression);
 private:
  // OpenPPL
  TPParseTreeNode ParseOpenEndedWhenConditionSequence();
  TPParseTreeNode ParseOpenPPLAction();
  TPParseTreeNode ParseOpenPPLRaiseToExpression();
  TPParseTreeNode ParseOpenPPLRaiseByExpression();
  TPParseTreeNode ParseOpenPPLUserVar();
 private:
  void BackPatchOpenEndedWhenConditionSequence(
    TPParseTreeNode first_when_condition_of_a_function);
 private:
  bool IsValidFunctionName(CString name);
  void ErrorMissingAction(int token_ID);
 private:
  CFormulaFileSplitter  _formula_file_splitter;
  CTokenizer            _tokenizer;
  CParseTreeRotator     _parse_tree_rotator;
  CParserSymbolTable    *_p_parser_symbol_table;
 private:
  CString _token;
  bool _is_parsing;
};

extern CFormulaParser *p_formula_parser;

#define PARSER p_formula_parser;

#endif INC_CFORMULAPARSER_H