#include "codegen.h"
#include <iostream>
#include <sstream>
using namespace std;

void CodeGen::_genPrologue() {
    // store parameters of wain
    cout << "; prologue" << endl;
    cout << "lis $4" << endl;
    cout << ".word 4" << endl;
    cout << "lis $11" << endl;
    cout << ".word 1" << endl;
    cout << "sub $29, $30, $4" << " ; $29 points to bottom of stack frame" << endl;
    cout << endl;
}

void CodeGen::_genEpilogue() {
    cout << endl << "; Epilogue" << endl;
    cout << "add $30, $29, $4" << " ; pop everything" << endl;
    cout << "jr $31" << endl;
}

// Generate the code for the parse tree t.
void CodeGen::genCode(tree *t) {
    // generate the code
    _genPrologue();

    //start BOF procedures EOF
    string currentProcedure = "";
    _genCode(t->children[1], currentProcedure);

    _genEpilogue();
}

void CodeGen::pushToStack(int reg, string symbolId) {
    cout << "; -- push -- " << "ptr: " << curStackPtr << endl;
    cout << "sw $" << reg << ", -4($30)" << " ; store " << symbolId << " on stack" << endl;
    cout << "sub $30, $30, $4" << " ; decrement stack pointer "<<endl;
    cout << endl;
    // update pointer
    curStackPtr -= 4;
}

void CodeGen::popToRegister(int reg) {
    cout << "; -- pop -- " << "ptr: " << curStackPtr << endl;
    cout << "lw $" << reg << ", 0($30)" << " ; pop from stack to register " << reg <<  endl;
    cout << "add $30, $30, $4" << "; increment stack pointer" << endl;
    cout << endl;
    curStackPtr += 4;
}

int stringToInt(string s) {
    istringstream iss(s);
    int n;
    iss >> n;
    return n;
}

void CodeGen::_genCode(tree *t, string currentProcedure) {
    if(t->rule == "procedures main") {
        _genCode(t->children[0], currentProcedure);
    }

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
        cout << endl;

        _genCode(t->children[8], currentProcedure); // dcls
        _genCode(t->children[9], currentProcedure); // statements
        _genCode(t->children[11], currentProcedure); // expr
    }

    // if(t->rule == "dcls") {}

    // if(t->rule == "statements") {}

    if (t->rule == "dcls dcls dcl BECOMES NUM SEMI") {
        tree *dcl = t->children[1]; // dcl type ID
        string symbol = dcl->children[1]->tokens[1];
        string val = t->children[3]->tokens[1];
        
        cout << endl;
        cout << "; CODE: int " << symbol << " = " << val << endl;
        cout << "lis $5" << endl;
        cout << ".word " << val << endl;

        // push $5 onto the stack
        pushToStack(5, symbol);
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

    if(t->rule == "factor ID") {
        string symbol = t->children[0]->tokens[1];
        int offset = SymbolTable::getInstance()->getOffset(currentProcedure, symbol);
        
        // return to $3
        cout << "; " << "Reached an ID(" << symbol << "), output to $3" << endl;
        cout << "lw $3, " << offset << "($29)" << endl;
    }

    if(t->rule == "factor LPAREN expr RPAREN") {
        _genCode(t->children[1], currentProcedure);
    }

    if(t->rule == "type INT") {}

    if(t->rule == "expr expr PLUS term") {
        _genCode(t->children[0], currentProcedure); // generate code for the expr
        pushToStack(3, "expr1");
        _genCode(t->children[2], currentProcedure); // generate code for the term
        
        string LType = t->children[0]->type;
        string RType = t->children[2]->type;

        // $5 has expr, $3 has term
        popToRegister(5);

        if (LType == "int*" && RType == "int") {
            cout << "; add int* with int" << endl;
            cout << "mult $3, $4" << endl;
            cout << "mflo $3" << " ; $3 <- 4*$3"<< endl;
            cout << "add $3, $3, $5" << endl;
        }
        else if (LType == "int" && RType == "int*") {
            cout << "; add int with int*" << endl;
            cout << "mult $5, $4" << endl;
            cout << "mflo $5" << "$5 <- 4*$5" <<endl;
            cout << "add $3, $3, $5";
        }
        else { //(LType == "int" && RType == "int") {
            cout << "add $3, $5, $3" << endl;
        }
    }
    if(t->rule == "expr expr MINUS term") {
        _genCode(t->children[0], currentProcedure); // generate code for the expr
        pushToStack(3, "expr1");
        _genCode(t->children[2], currentProcedure); // generate code for the term
        popToRegister(5);

        // $5 has expr, $3 has term
        cout << "sub $3, $5, $3" << endl;
    }
    if(t->rule == "term term STAR factor") {
        _genCode(t->children[0], currentProcedure); // generate code for the expr
        pushToStack(3, "expr1");
        _genCode(t->children[2], currentProcedure); // generate code for the term
        popToRegister(5);

        cout << "mult $5, $3" << endl;
        cout << "mflo $3" << endl;
    }
    if(t->rule == "term term SLASH factor") {

    }
    if(t->rule == "term term PCT factor") {

    }
    if(t->rule == "factor NUM") {
        
    }
    
}