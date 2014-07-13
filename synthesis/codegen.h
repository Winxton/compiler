#ifndef __CODEGEN_H__
#define __CODEGEN_H__

#include "typechecker.h"

class CodeGen {
public:
        CodeGen() : curStackPtr(0), labelCounter(0) {}
	void genCode(tree *t);
private:
	int curStackPtr;
	int labelCounter;
	void _genCode(tree *t, std::string currentProcedure);
	void _genEpilogue();
	void _genPrologue();
	void pushToStack(int reg, std::string symbolId);
	void popToRegister(int reg);
};

#endif
