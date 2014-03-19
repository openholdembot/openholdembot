#ifndef INC_TOKENIZERCONSTANTS_H
#define INC_TOKENIZERCONSTANTS_H

enum 
{
	// End of file
	kTokenEndOfFile = 0,
	kTokenEndOfFunction,
	// Operators
	kTokenOperatorPlus,               // -
	kTokenOperatorMinus,              // +
	kTokenOperatorMultiplication,     // *
	kTokenOperatorDivision,           // /
	kTokenOperatorModulo,             // % (OpenHoldem)
	kTokenOperatorExponentiation,     // **
	kTokenOperatorLog,                // ln
	kTokenOperatorEquality,           // == (OpenHoldem) = (OpenPPL)
	kTokenOperatorApproximatellyEqual,// ~~ 
	kTokenOperatorSmaller,            // <
	kTokenOperatorSmallerOrEqual,     // <= 
	kTokenOperatorGreater,            // >
	kTokenOperatorGreaterOrEqual,     // >=
	kTokenOperatorNotEqual,           // !=
	kTokenOperatorLogicalAnd,         // &&
	kTokenOperatorLogicalOr,	      // ||
	kTokenOperatorLogicalNot,         // !
	kTokenOperatorLogicalXOr,         // ^^
	kTokenOperatorBinaryAnd,          // &
	kTokenOperatorBinaryOr,           // |
	kTokenOperatorBinaryNot,          // ~
	kTokenOperatorBinaryXOr,          // ^
	kTokenOperatorBitShiftLeft,       // <<
	kTokenOperatorBitShiftRight,      // >>
	kTokenOperatorBitCount,           // `	
	kTokenOperatorPercentage,         // % (OpenPPL)
	kTokenOperatorConditionalIf,      // ?
	kTokenOperatorConditionalElse,    // :
	kTokenOperatorConditionalWhen,    // WHEN
	// Brackets
	kTokenBracketOpen_1,              // (				
	kTokenBracketOpen_2,              // [
	kTokenBracketOpen_3,              // {
	kTokenBracketClose_1,             // )
	kTokenBracketClose_2,             // ]
	kTokenBracketClose_3,             // }
	// Numbers, Identifier, Cards
	kTokenIdentifier,
	kTokenNumber,
	kTokenPocketCards,
	// ## as start and end of list and function-headers
	kTokenDoubleShebang,              // ##
	// OpenPPL actions
	kTokenActionBeep,
	kTokenActionFold,
	kTokenActionCheck,
	kTokenActionCall,
	kTokenActionRaise,
	kTokenActionRaiseHalfPot,
	kTokenActionRaisePot,
	kTokenActionRaiseMax,
	kTokenActionReturn,
	kTokenActionSitOut,
	kTokenActionLeave,
	kTokenActionClose,
	// OpenPPL keyword FORCE
	kTokenKeywordForce,
	// Shanky-style delay (unsupported)
	kTokenUnsupportedDelay,
	// Always leave that at the very end
	kNumberOfTokens,
};

inline bool TokenIsBracketOpen(int token)
{
	return ((token == kTokenBracketOpen_1)
		|| (token == kTokenBracketOpen_2)
		|| (token == kTokenBracketOpen_3));
}

bool TokenIsUnary(int token);
bool TokenIsBinary(int token);
inline bool TokenIsTernary(int token)
{
	return ((token == kTokenOperatorConditionalIf)
		|| (token == kTokenOperatorConditionalWhen));
}


inline bool TokenIsBracketClose(int token)
{
	return ((token == kTokenBracketClose_1)
		|| (token == kTokenBracketClose_2)
		|| (token == kTokenBracketClose_3));
}

inline bool TokenIsOpenPPLAction(int token)
{
	return ((token >= kTokenActionBeep)
		&& (token <= kTokenActionClose));
}

#endif INC_TOKENIZERCONSTANTS_H