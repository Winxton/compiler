#ifndef __CODEGEN_H__
#define __CODEGEN_H__

#include "typechecker.h"

class CodeGen {
public:
	static void genCode(tree *t, TopSymbolTable &topSymbolTable);
private:
	static void _genCode(tree *t, TopSymbolTable &topSymbolTable);
	static void _genEpilogue();
	static void _genPrologue();
};

#endif