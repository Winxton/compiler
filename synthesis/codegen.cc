#include "codegen.h"
#include <iostream>
#include <sstream>
using namespace std;

void CodeGen::_genPrologue() {
    // store parameters of wain
    cout << "; prologue" << endl;
    cout << "lis $4" << endl;
    cout << ".word 4" << endl;
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

void CodeGen::pushToStack(string currentProcedure, int reg, string symbolId) {
    cout << "sw $" << reg << ", -4($30)" << " ; store " << symbolId << " on stack" << endl;
    cout << "sub $30, $30, $4" << endl;

    SymbolTable::getInstance()->setSymbolOffset(currentProcedure, symbolId, curStackPtr);
    // update pointer
    curStackPtr -= 4;
}

void CodeGen::popToRegister() {

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
        pushToStack(currentProcedure, 1, symbolA);
        // push $2
        pushToStack(currentProcedure, 2, symbolB);

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
        pushToStack(currentProcedure, 5, symbol);
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
        cout << "; " << t->rule << endl;
        cout << "lw $3, " << offset << "($29)" << endl;
    }

    if(t->rule == "type INT") {
    }


}