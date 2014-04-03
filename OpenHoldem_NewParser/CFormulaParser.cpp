#include "stdafx.h"
#include "CFormulaParser.h"

#include <assert.h>
#include "CFunction.h"
#include "CFunctionCollection.h"
#include "CParseErrors.h"
#include "CPreferences.h"
#include "TokenizerConstants.h"

CFormulaParser *p_formula_parser = NULL;

// Global for static accessor-function
CString current_function_name;

CFormulaParser::CFormulaParser()
{
	_is_parsing = false;
}

CFormulaParser::~CFormulaParser()
{}

// OK
void CFormulaParser::ParseFile(CArchive& formula_file)
{
	_is_parsing = true;
	while (true)
	{
		CString function_content = _formula_file_splitter.GetNextFunctionOrList(formula_file);
		if (function_content.GetLength() < 2)
		{
			write_log(preferences.debug_parser(), 
				"[Parser] Empty function received. Parse finished.\n");
			goto ExitLoop;
		}
		ParseSingleFormula(function_content);
	}
ExitLoop:
	_is_parsing = false;
}

// OK
bool CFormulaParser::ExpectDoubleShebangAsStartOfFunctionHeader()
{
	int _token_ID = _tokenizer.GetToken();
	if (_token_ID == kTokenIdentifier)
	{
		// Most common error: identifier instead of function header
		// This can only happen before the first function
		_token = _tokenizer.GetTokenString();
		_token = _token.MakeLower();
		if (_token.Left(6) == "custom")
		{
			CParseErrors::Error("Superfluous keyword custom");
			return false;
		}
		else if ((_token.Left(7) == "preflop")
			|| (_token.Left(4) == "flop")
			|| (_token.Left(4) == "turn")
			|| (_token.Left(5) == "river"))
		{
			CParseErrors::Error("Shanky-style betrounds. OpenHoldem-style ##f$function## expected");
			return false;
		}
		else if (_token.Left(3) == "new")
		{
			CParseErrors::Error("Old-style OpenPPL function. OpenHoldem-style ##f$function## expected");
			return false;
		}
		else
		{
			CParseErrors::Error("Shanky-style option settings?");
			return false;
		}
	}
	else if (_token_ID != kTokenDoubleShebang)
	{
		// Other errors
		CParseErrors::Error("Expecting a ##f$function## or ##listNNN##");
		return false;
	}
	return true;
}

// OK
bool CFormulaParser::ExpectDoubleShebangAsEndOfFunctionHeader()
{
	_token_ID = _tokenizer.GetToken();
	if (_token_ID != kTokenDoubleShebang)
	{
		CParseErrors::Error("Malformed function-header. ## expected");
		return false;
	}
	return true;
}

// OK
bool CFormulaParser::ExpectConditionalThen()
{
	_token_ID = _tokenizer.GetToken();
	if (_token_ID != kTokenOperatorConditionalElse)
	{
		CParseErrors::Error("Malformed conditional expression. \":\" expected, but this could also be a missing operator or wrong bracket.");
		return false;
	}
	return true;
}

// OK
void CFormulaParser::CheckForExtraTokensAfterEndOfFunction()
{
	_token_ID = _tokenizer.GetToken();
	if ((_token_ID != kTokenEndOfFile)
		&& (_token_ID != kTokenEndOfFunction))
	{
		CParseErrors::Error("Unexpected token(s) after end of function");
	}
	// Nothing more to do here, not even returning a result.
	// We are finished and just warn about the extra input.
}

// OK
void CFormulaParser::ExpectMatchingBracketClose(int opening_bracket)
{
	assert(TokenIsBracketOpen(opening_bracket));
	int expected_bracket_close = _tokenizer.GetToken();
	switch (opening_bracket)
	{
	case kTokenBracketOpen_1:
		if (expected_bracket_close == kTokenBracketClose_1) return;
	case kTokenBracketOpen_2:
		if (expected_bracket_close == kTokenBracketClose_2) return;
	case kTokenBracketOpen_3:
		if (expected_bracket_close == kTokenBracketClose_3) return;
	}
	CParseErrors::Error("Expecting closing bracket (of another type)"); 
}

// OK
void CFormulaParser::ParseSingleFormula(CString function_text)
{
	current_function_name = "-- undefined --";
	_tokenizer.SetInput(function_text);

	// Check for empty function
	_token_ID = _tokenizer.GetToken();
	if ((_token_ID == kTokenEndOfFile)
		|| (_token_ID == kTokenEndOfFunction))
	{
		// Parsing finished (for this function or altogether)
		return;
	}
	else
	{
		_tokenizer.PushBack();
	}
	// Check correct start of function header 
	// and especially throw warnings on old-style and Shanky-style PPL.
	if (!ExpectDoubleShebangAsStartOfFunctionHeader())
	{
		return;
	}
	_token_ID = _tokenizer.GetToken();
	if (_token_ID == kTokenNumber)
	{
		// Date like ##2014-02-09 23:16:55##
		// To be completely ignored
		// We don't need it and on saving we create a new one
		write_log(preferences.debug_parser(), 
			"[Parser] Found a ##number(##). Probably date. To be ignored.\n");
		return;
	}
	else if (_token_ID != kTokenIdentifier)
	{
		CParseErrors::Error("Malformed function-header. Name expected");
		return;
	}
	_token = _tokenizer.GetTokenString();
	current_function_name = _token;
	TPParseTreeNode function_body = NULL;
	if (_token.Left(2) == "f$")
	{
		// ##f$functionXYZ##
		write_log(preferences.debug_parser(), 
				"[Parser] Parsing f$function\n");
		if (!ExpectDoubleShebangAsEndOfFunctionHeader())
		{
			return;
		}
		TPParseTreeNode function_body =	ParseFunctionBody();
	}
	else if (_token.Left(4) == "list")
	{
		// ##listNNN##
		write_log(preferences.debug_parser(), 
				"[Parser] Parsing list\n");
		ExpectDoubleShebangAsEndOfFunctionHeader();
		ParseListBody();
	}
	else if (_token.MakeLower() == "dll")
	{
		// ##DLL##
		write_log(preferences.debug_parser(), 
			"[Parser] Parsing ##DLL##\n");
		ExpectDoubleShebangAsEndOfFunctionHeader();
		// !!! To do
	}
	else if (_token.MakeLower() == "notes")
	{
		// ##Notes##
		write_log(preferences.debug_parser(), 
			"[Parser] Found ##notes##. Nothing to parse\n");
		ExpectDoubleShebangAsEndOfFunctionHeader();
		// Don't do anything.
		// This is just a special type of global comment.
	}
	else
	{
		CParseErrors::Error("Found unknown function type. Did you forget f$?\n");
		return;
	}
	CheckForExtraTokensAfterEndOfFunction();
	CFunction *p_new_function = new CFunction(current_function_name, 
		function_text);
	p_new_function->SetParseTree(function_body);
	p_function_collection->Add(p_new_function);
}

// Nearly OK
void CFormulaParser::ParseListBody()
{
	_token_ID = _tokenizer.GetToken();
	while (_token_ID != kTokenEndOfFunction)
	{
		if ((_token_ID == kTokenIdentifier)      // High cards (at least one), like AK2 T2o
			|| (_token_ID == kTokenNumber)       // Low pairs 99..22
			|| (_token_ID == kTokenPocketCards)) // Low unpaired cards like 65s, 92o
		{
			_token = _tokenizer.GetTokenString();
			// !!! ToDo: Add to list
		}
		else
		{
			CParseErrors::Error("Unexpected token inside list");
			return;
		}
		_token_ID = _tokenizer.GetToken();
	}
}


// OK
TPParseTreeNode CFormulaParser::ParseFunctionBody()
{
	// Just look-ahead 1 token
	_token_ID = _tokenizer.LookAhead();
	if ((_token_ID == kTokenEndOfFile) 
		|| (_token_ID == kTokenEndOfFunction))
	{
		// Empty function; evaluating to zero
		TPParseTreeNode terminal_node = new CParseTreeNode();
		terminal_node->MakeConstant(0);
		return terminal_node;
	}
	if (_token_ID == kTokenOperatorConditionalWhen)
	{
		// OpenPPL-function
		return ParseOpenEndedWhenConditionSequence();
	}
	else
	{	
		// OH-script-function, single expression
		return ParseExpression();
	}
}

// OK
TPParseTreeNode CFormulaParser::ParseExpression()
{
	_token_ID = _tokenizer.LookAhead();
	TPParseTreeNode expression;
	if (TokenIsUnary(_token_ID))
	{
		expression = ParseUnaryExpression();
	}
	else if (TokenIsBracketOpen(_token_ID))
	{
		expression = ParseBracketExpression();
	}
	else if ((_token_ID == kTokenIdentifier)
		|| (_token_ID == kTokenNumber))
	{
		expression = ParseSimpleExpression();
	}
	else
	{
		CParseErrors::Error("Unexpected token inside expression. Expect: Opening Bracket, Unary, Identifier or number");
		return NULL;
	}
	_token_ID = _tokenizer.LookAhead();
	if (TokenIsBinary(_token_ID))
	{
		_tokenizer.GetToken();
		TPParseTreeNode second_expression = ParseExpression();
		TPParseTreeNode binary_node = new CParseTreeNode();
		binary_node->MakeBinaryOperator(_token_ID, 
			expression, second_expression);
		return binary_node;
	}
	else if (_token_ID == kTokenOperatorConditionalIf)
	{
		// Binary or ternary condition!!!
		TPParseTreeNode then_expression;
		TPParseTreeNode else_expression;
		ParseConditionalPartialThenElseExpressions(
			&then_expression, &else_expression);
		TPParseTreeNode ternary_node = new CParseTreeNode();
		ternary_node->MakeTernaryOperator(_token_ID,
			expression, then_expression, else_expression);
		return ternary_node;
	}
	else
	{
		// We got the complete expression
		// No complex binary or ternary condition
		return expression;
	}
}

// OK
TPParseTreeNode CFormulaParser::ParseBracketExpression()
{
	//MessageBox(0, "Bracket_Open", "Info", 0);
	int opening_bracket = _tokenizer.GetToken();
	assert(TokenIsBracketOpen(opening_bracket));
	TPParseTreeNode expression = ParseExpression();
	ExpectMatchingBracketClose(opening_bracket);
	TPParseTreeNode bracket_node = new CParseTreeNode();
	// Brackets get an unary node in the tree
	// This will lead to a simple way to handle precedence of operators.
	bracket_node->MakeUnaryOperator(opening_bracket, expression);
	return bracket_node;
}

// OK
TPParseTreeNode CFormulaParser::ParseUnaryExpression()
{
	//MessageBox(0, "Unary", "Info", 0);
	int unary_operator = _tokenizer.GetToken();
	assert(TokenIsUnary(unary_operator));
	TPParseTreeNode expression = ParseExpression();
	TPParseTreeNode unary_node = new CParseTreeNode();
	unary_node->MakeUnaryOperator(unary_operator, expression);
	return unary_node;
}

// OK
TPParseTreeNode CFormulaParser::ParseSimpleExpression()
{
	int terminal = _tokenizer.GetToken();
	assert((terminal == kTokenIdentifier) || (terminal == kTokenNumber));
	TPParseTreeNode terminal_node = new CParseTreeNode();
	if (terminal == kTokenIdentifier)
	{
		terminal_node->MakeIdentifier(_tokenizer.GetTokenString());
	}
	else if (terminal == kTokenNumber)
	{
		CString number = _tokenizer.GetTokenString();
		double value = atof(number);
		terminal_node->MakeConstant(value);
	}
	else
	{
		assert(k_this_must_not_happen);
		terminal_node = NULL;	
	}
	return terminal_node;
}

// OK
void CFormulaParser::ParseConditionalPartialThenElseExpressions(
	TPParseTreeNode *then_expression, TPParseTreeNode *else_expression)
{
	// <Condition> ? <Then-Expression> : <Else-Expression>
	// Condition up to question-mark already parsed
	int token_ID = _tokenizer.GetToken();
	assert(token_ID == kTokenOperatorConditionalIf);
	*then_expression = ParseExpression();
	if (!ExpectConditionalThen())
	{
		*else_expression = NULL;
	}
	else
	{
		*else_expression = ParseExpression();
	}
	// Results get returned via out-pointers here.
	// We have to put everything together on the call-site.
}

// OK
CString CFormulaParser::CurrentFunctionName()
{
	return current_function_name;
}

TPParseTreeNode CFormulaParser::ParseOpenEndedWhenConditionSequence()
{
	//MessageBox(0, "When", "Info", 0);
	_token_ID = _tokenizer.GetToken();
	assert(_token_ID == kTokenOperatorConditionalWhen);
	ParseExpression();
	// Next either:
	// * another when-condition
	// * action
	// * user-variable to be set
	_token_ID = _tokenizer.LookAhead();
	if (_token_ID == kTokenOperatorConditionalWhen)
	{
		ParseOpenEndedWhenConditionSequence();
	}
	else if (TokenIsOpenPPLAction(_token_ID))
	{
		ParseOpenPPLAction();
	}
	else 
	{
		CParseErrors::Error("Missing action after when condition");
	}
	_token_ID = _tokenizer.LookAhead();
	if (_token_ID == kTokenOperatorConditionalWhen)
	{
		ParseOpenEndedWhenConditionSequence();
	}
	return NULL;
}

// OK
TPParseTreeNode CFormulaParser::ParseOpenPPLAction()
{
	//MessageBox(0, "Action", "Info", 0);
	_token_ID = _tokenizer.GetToken();
	assert(TokenIsOpenPPLAction(_token_ID));
	TPParseTreeNode action;
	if (_token_ID == kTokenActionReturn)
	{
		// RETURN <Expression> FORCE
		action = ParseExpression();
	}
	else if (_token_ID == kTokenActionRaise)
	{
		// There are 3 possibilities
		//   RAISE FORCE
		//   RAISE <Amount> FORCE
		//   RAISE <PercentagedPot>% FORCE
		action = ParseOpenPPLRaiseExpression();
	}
	else if (_token_ID == kTokenIdentifier)
	{
		// User-variable to be set
		CString identifier = _tokenizer.GetTokenString();
		if (identifier.Left(4).MakeLower() != "user")
		{
			CParseErrors::Error("Unexpected identifier. Action expected");
			return NULL;
		}
		else
		{
			action = new CParseTreeNode();
			action->MakeUserVariableDefinition(identifier);
		}
	}
	else 
	{
		// Something completely unexpected
		CParseErrors::Error("Unexpected token. Action expected");
		return NULL;
	}
	ExpectKeywordForce();
	return action;
}

// OK
bool CFormulaParser::ExpectKeywordForce()
{
	int _token_ID = _tokenizer.GetToken();
	if (_token_ID == kTokenKeywordForce)
	{
		// Check for unsupported Shanky-style delay
		// which can only happen after actions 
		// WHEN ... RAISEMAX FORCE DELAY 42
		_token_ID = _tokenizer.LookAhead();
		if (_token_ID == kTokenUnsupportedDelay)
		{
			CParseErrors::Error("Unsupported Shanky-style delay");
			// And consume 2 tokens to avoid further messages;
			_token_ID = _tokenizer.GetToken();
			_token_ID = _tokenizer.GetToken();
		}
		// Both cases, with and without delay, are considered "good".
		return true;
	}
	else
	{
		CParseErrors::Error("Missing keyword force");
		return false;
	}
}

// OK
TPParseTreeNode CFormulaParser::ParseOpenPPLRaiseExpression()
{
	// There are 3 possibilities
	//   RAISE FORCE
	//   RAISE <Amount> FORCE
	//   RAISE <PercentagedPot>% FORCE
	//
	// Keyword RAISE got already consumed
	TPParseTreeNode action = new CParseTreeNode();
	TPParseTreeNode expression;
	int _token_ID = _tokenizer.LookAhead();
	if ((_token_ID == kTokenNumber)
		|| (_token_ID == kTokenIdentifier)
		|| TokenIsBracketOpen(_token_ID))
	{
		expression = ParseExpression();
	}
	else
	{
		// Simple RAISE
		action->MakeAction(kTokenActionRaise);
		return action;
	}
	_token_ID = _tokenizer.LookAhead();
	if (_token_ID == kTokenOperatorPercentage)
	{
		// Percentaged Potsize
		_tokenizer.GetToken();
		action->MakeRaiseByPercentagedPotsizeAction(expression);
		return action;
	}
	else
	{
		// Raise by N big blinds
		action->MakeRaiseByAction(expression);
		return action;
	}
	return NULL;
}
