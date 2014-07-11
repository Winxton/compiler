#ifndef __TYPECHECKER_H__
#define __TYPECHECKER_H__

#include "symboltable.h"
#include <string>
#include <vector>
#include <map>

// procedure name - pair with (signature , map <symbol, pair containing (type, offset))
typedef std::pair<std::vector<std::string>, std::map<std::string, std::string> > SignatureSymbolTablePair;
typedef std::map<std::string, SignatureSymbolTablePair > TopSymbolTable;

// Data structure for storing the parse tree.
struct tree {
    std::string rule;
    std::string type;
    std::vector<std::string> tokens;
    std::vector<tree*> children;
    ~tree() { for(int i=0; i<children.size(); i++) delete children[i]; }
};

class TypeChecker {
public:
	// generate the symbol table
	static void genSymbols(tree *t, std::string currentProcedure);

	// returns the type of a node
	static std::string getType(tree *node, std::string currentProcedure);

private:
	static std::pair<std::string, std::string> getNameAndType(tree *t);

	// fills the signature vector given the tree node
	static void populateSignatures(tree *t, std::vector<std::string> &signature);

	// returns an argument list from the tree node
	static void populateArgList(tree *t, std::string currentProcedure, std::vector<std::string> &argList);

	// assert a node is well typed
	static void assertWellTyped(const tree *node, std::string currentProcedure);

	// returns the procedure name
	static std::string getProcedureName(tree *t);
};

#endif