#ifndef __TYPECHECKER_H__
#define __TYPECHECKER_H__

#include "symboltable.h"
#include <string>
#include <vector>
#include <map>

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