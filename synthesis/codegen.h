#ifndef __CODEGEN_H__
#define __CODEGEN_H__

#include "typechecker.h"

class CodeGen {
public:
	CodeGen() : curStackPtr(0){}
	void genCode(tree *t);
private:
	int curStackPtr;
	void _genCode(tree *t, std::string currentProcedure);
	void _genEpilogue();
	void _genPrologue();
	void pushToStack(std::string currentProcedure, int reg, std::string symbolId);
	void popToRegister();
};

#endif