#include "codegen.h"
#include <iostream>
#include <sstream>
using namespace std;

void CodeGen::_genPrologue() {
    // store parameters of wain
    cout << "; prologue" << endl;
    cout << ".import init" << endl;
    cout << ".import new" << endl;
    cout << ".import delete" << endl;
    cout << ".import print" << endl;
    cout << "lis $4" << endl;
    cout << ".word 4" << endl;
    cout << "lis $11" << endl;
    cout << ".word 1" << endl;
    cout << "sub $29, $30, $4" << " ; $29 points to bottom of stack frame" << endl;

    // call init
    cout << "; call init" << endl;
    pushToStack(31, "PC");
    cout << "lis $5" << endl;
    cout << ".word init" << endl;
    cout << "jalr $5" << endl;
    popToRegister(31);
    
    cout << endl;
}

// Generate the code for the parse tree t.
void CodeGen::genCode(tree *t) {
    // generate the code
    _genPrologue();

    //start BOF procedures EOF
    string currentProcedure = "";
    _genCode(t->children[1], currentProcedure);

}

void CodeGen::pushToStack(int reg, string symbolId) {
    cout << "; PUSH " << symbolId << " to stack" << ", ptr: " << curStackPtr << endl;
    cout << "sw $" << reg << ", -4($30)" << endl;
    cout << "sub $30, $30, $4" << " ; decrement stack pointer "<<endl;
    // update pointer
    curStackPtr -= 4;
}

void CodeGen::popToRegister(int reg) {
    cout << "; POP to register " << reg << ", ptr: " << curStackPtr << endl;
    cout << "lw $" << reg << ", 0($30)" << endl;
    cout << "add $30, $30, $4" << "; increment stack pointer" << endl;
    curStackPtr += 4;
}

int stringToInt(string s) {
    istringstream iss(s);
    int n;
    iss >> n;
    return n;
}

string intToString(int n) {
    stringstream oss;
    oss << n;
    return oss.str();
}


void CodeGen::_genCode(tree *t, string currentProcedure) {
    if(t->rule == "procedures main") {
        _genCode(t->children[0], currentProcedure);

        // epilogue
        cout << endl << "; Epilogue" << endl;
        cout << "add $30, $29, $4" << " ; pop everything" << endl;
        cout << "jr $31" << endl;
    }

    if (t->rule == "procedures procedure procedures") {
        // reverse the orders
        _genCode(t->children[1], currentProcedure);
        _genCode(t->children[0], currentProcedure);
    }
    
    //procedure definition
    if (t->rule == "procedure INT ID LPAREN params RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE") {
        // label for the procedure
        string procName = t->children[1]->tokens[1];
        cout << "; starting definition for function : " << procName << endl;
        string label = "F" + procName;
        cout << label << ":" << endl;

        // update stack frame
        int tempCurStackPtr = curStackPtr;
        curStackPtr = 0; // reset stack ptr for procedure
        cout << "sub $29, $30, $4" << endl; // update $29 to just after arguments and before local vars

        // TODO: add # of params to symbol table
        _genCode(t->children[3], procName); // params

        cout << "; " << procName << " declarations: " << endl;
        _genCode(t->children[6], procName); // dcls

        _genCode(t->children[7], procName); // statements
        _genCode(t->children[9], procName); // expr (return)


        curStackPtr = tempCurStackPtr; // restore stack ptr
        cout << "add $30, $29, $4" << " ; restore stack ptr" << endl;
        cout << "jr $31" << endl;
    }

    // procedure call with no params
    if (t->rule == "factor ID LPAREN RPAREN") {
        string label = "F" + t->children[0]->tokens[1];
        pushToStack(29, "stack frame");
        pushToStack(31, "PC");

        cout << "lis $5" << endl;
        cout << ".word " << label << endl;
        cout << "jalr $5 " << endl;
        popToRegister(31);
        popToRegister(29);
    }

    // wain declaration
    if(t->rule == "main INT WAIN LPAREN dcl COMMA dcl RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE") {
        currentProcedure = "wain";
        string symbolA = t->children[3]->children[1]->tokens[1];
        string symbolB = t->children[5]->children[1]->tokens[1];

        // push $1
        SymbolTable::getInstance()->setSymbolOffset(currentProcedure, symbolA, curStackPtr);
        pushToStack(1, symbolA);
        // push $2
        SymbolTable::getInstance()->setSymbolOffset(currentProcedure, symbolB, curStackPtr);
        pushToStack(2, symbolB);

        _genCode(t->children[8], currentProcedure); // dcls
        _genCode(t->children[9], currentProcedure); // statements
        _genCode(t->children[11], currentProcedure); // expr
    }

    // if(t->rule == "dcls") {}
    // if(t->rule == "statements") {}

    if(t->rule == "statements statements statement") {
      _genCode(t->children[0], currentProcedure);
      _genCode(t->children[1], currentProcedure);
    }

    if(t->rule == "statements statement") {
      _genCode(t->children[0], currentProcedure);
    }

    if(t->rule == "statement PRINTLN LPAREN expr RPAREN SEMI") {
      _genCode(t->children[2], currentProcedure); // code for expr
      cout << "; CALL PRINT (clobbers $1, assume not required to keep orig value) " << endl;
      cout << "add $1, $3, $0" << endl;
      pushToStack(31, " stack pointer "); // store end of program pointer
      cout << "lis $5" << endl;
      cout << ".word print" << endl;
      cout << "jalr $5" << endl;
      popToRegister(31); // restore end of program pointer
    }
    
    // set variable to int
    if (t->rule == "dcls dcls dcl BECOMES NUM SEMI") {
        _genCode(t->children[0], currentProcedure);

        tree *dcl = t->children[1]; // dcl type ID
        string symbol = dcl->children[1]->tokens[1];
        string val = t->children[3]->tokens[1];
        
        cout << "; dcl: int " << symbol << " = " << val << endl;
        cout << "lis $5" << endl;
        cout << ".word " << val << endl;

        // push $5 onto the stack
        SymbolTable::getInstance()->setSymbolOffset(currentProcedure, symbol, curStackPtr);
        pushToStack(5, symbol);
    }

    // set variable to NULL
    if (t->rule == "dcls dcls dcl BECOMES NULL SEMI") {
        _genCode(t->children[0], currentProcedure);

        tree *dcl = t->children[1]; // dcl type ID
        string symbol = dcl->children[1]->tokens[1];
        string val = t->children[3]->tokens[1];
        
        cout << "; dcl: int * " << symbol << " = " << val << endl;
        cout << "lis $5" << endl;
        // Use 1 as NULL
        cout << ".word " << 1 << endl;

        // push $5 onto the stack
        SymbolTable::getInstance()->setSymbolOffset(currentProcedure, symbol, curStackPtr);
        pushToStack(5, symbol);
    }

    if (t->rule == "factor NEW INT LBRACK expr RBRACK") {
        cout << "; NEW expr" << endl;
        _genCode(t->children[3], currentProcedure);
        cout << "; actual NEW calls" << endl;

        string skipLabel = "skip" +intToString(labelCounter);
        ++ labelCounter;

        cout << "add $1, $3, $0 ; # of words needed" << endl;

        pushToStack(31, "return ptr");
        cout << "lis $5" << endl;
        cout << ".word new" << endl;
        cout << "jalr $5" << endl;
        popToRegister(31);
        // if new returns 0, replace with 0x11
        cout << "bne $3, $0, " << skipLabel << " ; not 0, skip " << endl;
        cout << "add $3, $0, $11" << " ; replace result with repr of NULL" << endl;
        cout << skipLabel << ":" << endl;
    }

    if (t->rule == "statement DELETE LBRACK RBRACK expr SEMI") {
        cout << "; DELETE expr" << endl;
        _genCode(t->children[3], currentProcedure);
        cout << "; actual DELETE calls " << endl;

        string nullLabel = "null" +intToString(labelCounter);
        ++ labelCounter;
        
        // if expr is NULL, skip DELETE
        cout << "beq $11, $3, " << nullLabel << " ; addr to deallocate is null(0x01)" << endl;

        cout << "add $1, $0, $3" << "; move ptr to mem to deallocate to $1 for delete subroutine" << endl;

        pushToStack(31, "PC");
        cout << "lis $5" << endl;
        cout << ".word delete" << endl;
        cout << "jalr $5" << endl;
        popToRegister(31);

        cout << nullLabel << ":" << endl;
    }
    
    if (t->rule == "factor NULL"){
        cout << "; Factor NULL" << endl;
        cout << "lis $3" << endl;
        cout << ".word 1" << endl;
    }
    if (t->rule == "factor AMP lvalue"){
        // get reference
        _genCode(t->children[1], currentProcedure);
    }
    if (t->rule == "factor STAR factor"){
        cout << "; " << t->rule << endl; 
        // dereference - factor on RHS is an lvalue
        _genCode(t->children[1], currentProcedure);
        cout << "lw $3, 0($3)" << endl; 
    }
    if (t->rule == "lvalue STAR factor"){
        // get reference
        _genCode(t->children[1], currentProcedure);
    }

    if (t->rule == "dcl type ID") {
        //string symbol = t->children[1]->tokens[1];
        // can't do anything here yet!
    }

    if (t->rule == "params paramlist") {
        _genCode(t->children[0], currentProcedure);
    }
    if (t->rule == "paramlist dcl") {
        _genCode(t->children[0], currentProcedure);
    }
    if (t->rule == "paramlist dcl COMMA paramlist") {
        _genCode(t->children[0], currentProcedure);
        _genCode(t->children[2], currentProcedure);
    }
    if(t->rule == "expr term") {
        _genCode(t->children[0], currentProcedure);
    }
    if(t->rule == "term factor") {
        _genCode(t->children[0], currentProcedure);
    }

    if(t->rule == "factor NUM") {
      string val = t->children[0]->tokens[1];
      cout << "; reached NUM( " << val << "), return to $3" << endl;
      cout << "lis $3" << endl;
      cout << ".word " << val << endl;
    }

    if(t->rule == "factor ID") {
        string symbol = t->children[0]->tokens[1];
        int offset = SymbolTable::getInstance()->getOffset(currentProcedure, symbol);
        
        // return to $3
        cout << "; " << "Reached an ID(" << symbol << "), output to $3" << endl;
        cout << "lw $3, " << offset << "($29)" << " ; load " << symbol << endl;
    }

    if(t->rule == "factor LPAREN expr RPAREN") {
        _genCode(t->children[1], currentProcedure);
    }

    if(t->rule == "type INT") {}

    if(t->rule == "type INT STAR") {}

    if (t->rule == "statement lvalue BECOMES expr SEMI") {
      _genCode(t->children[0], currentProcedure); // lvalue
      pushToStack(3, "lvalue");
      _genCode(t->children[2], currentProcedure); // expr
      popToRegister(5);
      // lvalue: $5, expr: $3
      cout << "sw $3, 0($5)" << "; Store result of new LVALUE " << t->rule << endl;
    }

    if (t->rule == "lvalue ID") {
      string symbol = t->children[0]->tokens[1];
      int offset = SymbolTable::getInstance()->getOffset(currentProcedure, symbol);
      cout << "; get address of lvalue ID(" << symbol << ")" << endl;
      cout << "lis $3" << endl;
      cout << ".word " << offset << endl;
      cout << "add $3, $3, $29" << "; location of symbol" << endl;
    }

    if (t->rule == "lvalue LPAREN lvalue RPAREN") {
      cout << "; " << t->rule << endl;
      _genCode(t->children[1], currentProcedure);
    }

    // TESTS
    if (t->rule == "test expr LT expr") {
      _genCode(t->children[0], currentProcedure);
      pushToStack(3, "test expr1");
      _genCode(t->children[2], currentProcedure);
      popToRegister(5);
      // use slt for int, sltu for int*
      string slt = t->children[0]->type == "int" ? "slt" : "sltu";
      // LHS: $5, RHS: $3
      cout << slt << " $3, $5, $3" << endl;
    }
    if (t->rule == "test expr EQ expr") {
      _genCode(t->children[0], currentProcedure);
      pushToStack(3, "test expr1");
      _genCode(t->children[2], currentProcedure);
      popToRegister(5);
      // LHS: $5, RHS: $3
      string slt = t->children[0]->type == "int" ? "slt" : "sltu";
      cout << slt << " $6, $5, $3" << endl;
      cout << slt << " $7, $3, $5" << endl;
      cout << "add $3, $6, $7" << endl;
      cout << "sub $3, $11, $3" << endl;
    }
    if (t->rule == "test expr NE expr") {
      _genCode(t->children[0], currentProcedure);
      pushToStack(3, "test expr1");
      _genCode(t->children[2], currentProcedure);
      popToRegister(5);
      // LHS: $5, RHS: $3
      string slt = t->children[0]->type == "int" ? "slt" : "sltu";
      cout << slt << " $6, $5, $3" << endl;
      cout << slt << " $7, $3, $5" << endl;
      cout << "add $3, $6, $7" << endl;
    }
    if (t->rule == "test expr LE expr") {
      _genCode(t->children[0], currentProcedure);
      pushToStack(3, "test expr1");
      _genCode(t->children[2], currentProcedure);
      popToRegister(5);
      // LHS: $5, RHS: $3
      string slt = t->children[0]->type == "int" ? "slt" : "sltu";

      cout << slt << " $3, $3, $5" << endl;
      cout << "sub $3, $11, $3" << endl;
    }
    if (t->rule == "test expr GE expr") {
      _genCode(t->children[0], currentProcedure);
      pushToStack(3, "test expr1");
      _genCode(t->children[2], currentProcedure);
      popToRegister(5);
      // LHS: $5, RHS: $3
      string slt = t->children[0]->type == "int" ? "slt" : "sltu";

      cout << slt << " $3, $5, $3" << endl;
      cout << "sub $3, $11, $3" << endl;
    }
    if (t->rule == "test expr GT expr") {
      _genCode(t->children[0], currentProcedure);
      pushToStack(3, "test expr1");
      _genCode(t->children[2], currentProcedure);
      popToRegister(5);

      // LHS: $5, RHS: $3
      string slt = t->children[0]->type == "int" ? "slt" : "sltu";
      
      cout << slt << " $3, $3, $5" << endl;
    }
    if (t->rule == "statement IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE"){
      string labelCounterStr = intToString(labelCounter);
      string endLabel = "end" + labelCounterStr;
      string trueLabel = "true" + labelCounterStr;
      
      ++ labelCounter;

      cout << "; IF STATEMENT" << endl;
      _genCode(t->children[2], currentProcedure);
      cout << "beq $11, $3, " << trueLabel << endl;
      // false
      _genCode(t->children[9], currentProcedure);
      cout << "beq $0, $0, " << endLabel << endl;
      // true
      cout << trueLabel << ":" << endl;
      _genCode(t->children[5], currentProcedure);
      // end 
      cout << endLabel << ":" << endl;
    }
    if (t->rule == "statement WHILE LPAREN test RPAREN LBRACE statements RBRACE") {
      
      cout << "; WHILE LOOP" << endl;
      string labelCounterStr = intToString(labelCounter);
      // label for loop 
      string loopLabel = "loop" + labelCounterStr; 
      string endLoopLabel = "endloop" + labelCounterStr;

      ++ labelCounter;

      cout << loopLabel << ":" << endl;
      cout << "; WHILE LOOP: CODE FOR TEST" << endl;
      _genCode(t->children[2], currentProcedure);
      cout << "bne $3, $11, " << endLoopLabel << " ; Test is false, endloop" << endl; // test is false
      cout << "; WHILE LOOP: CODE FOR STATEMENT" << endl;
      _genCode(t->children[5], currentProcedure);
      cout << "beq $0, $0, " << loopLabel << endl;
      cout << endLoopLabel << ":" << endl;
    }

    if(t->rule == "expr expr PLUS term") {
        _genCode(t->children[0], currentProcedure); // generate code for the expr
        pushToStack(3, "expr1");
        _genCode(t->children[2], currentProcedure); // generate code for the term
        popToRegister(5);

        string LType = t->children[0]->type;
        string RType = t->children[2]->type;

        // $5 has expr, $3 has term
        if (LType == "int*" && RType == "int") {
            cout << "; add int* with int" << endl;
            cout << "mult $3, $4" << endl;
            cout << "mflo $3" << " ; $3 <- 4*$3"<< endl;
            cout << "add $3, $3, $5" << endl;
        }
        else if (LType == "int" && RType == "int*") {
            cout << "; add int with int*" << endl;
            cout << "mult $5, $4" << endl;
            cout << "mflo $5" << "; $5 <- 4*$5" <<endl;
            cout << "add $3, $3, $5";
        }
        else { //(LType == "int" && RType == "int") {
            cout << "; add int with int" << endl;
            cout << "add $3, $5, $3" << endl;
        }
    }

    if(t->rule == "expr expr MINUS term") {

        _genCode(t->children[0], currentProcedure); // generate code for the expr
        pushToStack(3, "expr1");
        _genCode(t->children[2], currentProcedure); // generate code for the term
        popToRegister(5);

        string LType = t->children[0]->type;
        string RType = t->children[2]->type;

        // LHS: $5 has expr, RHS: $3 has term
        if (LType == "int*" && RType == "int") {
            cout << "; int* - int" << endl;
            cout << "mult $3, $4" << endl; 
            cout << "mflo $3" << " ; $3 <- 4*$3"<< endl;
            cout << "sub $3, $5, $3" << " ; $3 <- $5 - 4*$3" << endl;
        }
        else if (LType == "int*" && RType == "int*") {
            cout << "; int* - int*" << endl;
            cout << "sub $3, $5, $3" << endl;
	    cout << "divu $3, $4" << " ; divide by 4 gives offset" << endl;
            cout << "mflo $3" << " ; $3 <- ($5 - $3)/4" << endl;
        }
        else { //(LType == "int" && RType == "int") {
	  cout << "sub $3, $5, $3" << endl;
        }

    }

    if(t->rule == "term term STAR factor") {
        _genCode(t->children[0], currentProcedure); // generate code for the expr
        pushToStack(3, "expr1");
        _genCode(t->children[2], currentProcedure); // generate code for the term
        popToRegister(5);

	cout << "; " << t->rule << endl;
        cout << "mult $5, $3" << endl;
        cout << "mflo $3" << endl;
    }

    if(t->rule == "term term SLASH factor") {
        _genCode(t->children[0], currentProcedure); // generate code for the expr
        pushToStack(3, "expr1");
        _genCode(t->children[2], currentProcedure); // generate code for the term
        popToRegister(5);

	cout << "; " << t->rule << endl;
        cout << "div $5, $3" << endl;
        cout << "mflo $3" << endl;
    }

    if(t->rule == "term term PCT factor") {
        _genCode(t->children[0], currentProcedure); // generate code for the expr
        pushToStack(3, "expr1");
        _genCode(t->children[2], currentProcedure); // generate code for the term
        popToRegister(5);
	
	cout << "; " << t->rule << endl;
        cout << "div $5, $3" << endl;
        cout << "mfhi $3" << endl;
    }

}
