#ifndef INC_CPARSETREENODE_H
#define INC_CPARSETREENODE_H

class CParseTreeNode;

typedef CParseTreeNode *TPParseTreeNode;

class CParseTreeNode
{
public:
	CParseTreeNode();
	~CParseTreeNode();
public:
	double Evaluate();
	double EvaluateIdentifier(CString name);
	double EvaluateUnaryExpression();
	double EvaluateBinaryExpression();
	double EvaluateTernaryExpression();
	double EvaluateSibbling(TPParseTreeNode first_second_or_third_sibbling);
private:
	int _node_type;
	// In case of terminal node (identifier)
	CString _terminal_name;
	// In case of terminal node (number)
	double _constant_value;
private:
	// In case of non-terminal-node
	TPParseTreeNode _first_sibbling;	// always present
	TPParseTreeNode _second_sibbling;	// for binary and ternary operators
	TPParseTreeNode _third_sibbling;	// for ternary operators only
};

#endif INC_CPARSETREENODE_H