#include "codegen.h"
#include <iostream>
#include <sstream>
using namespace std;

void CodeGen::_genPrologue() {
    ostringstream oss;

    // store parameters of wain
    oss << "; prologue" << endl;
    oss << "lis $4" << endl;
    oss << ".word 4" << endl;
    oss << "sub $29, $30, $4" << " ; $29 points to bottom of stack frame" << endl;
    oss << "sw $1, -4($30)" << " ; store a on stack" << endl;
    oss << "sub $30, $30, $4" << endl;
    oss << "sw $2, -4($30)" << " ; store b on stack" << endl;
    oss << "sub $30, $30, $4" << endl;

    cout << oss.str() << endl;
}

void CodeGen::_genEpilogue() {
    ostringstream oss;
    oss << "; Epilogue" << endl;
    oss << "add $30, $29, $4" << " ; pop everything" << endl;
    oss << "jr $31" << endl;

    cout << oss.str() << endl;
}

// Generate the code for the parse tree t.
void CodeGen::genCode(tree *t, TopSymbolTable &topSymbolTable) {
    // generate the code
    _genPrologue();

    

    //start BOF procedures EOF
    _genCode(t->children[1], topSymbolTable);
    _genEpilogue();
}

void CodeGen::_genCode(tree *t, TopSymbolTable &topSymbolTable) {
    if(t->rule == "procedures main") {
        genCode(t->children[0], topSymbolTable);
    }

    if(t->rule == "main INT WAIN LPAREN dcl COMMA dcl RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE") {
        genCode(t->children[8], topSymbolTable); // dcls
        genCode(t->children[9], topSymbolTable); // statements
        genCode(t->children[11], topSymbolTable); // expr
    }

    // if(t->rule == "dcls") {}

    // if(t->rule == "statements") {}

    if (t->rule == "dcl type ID") {

    }

    if(t->rule == "expr term") {
        genCode(t->children[0], topSymbolTable);
    }

    if(t->rule == "term factor") {
        genCode(t->children[0], topSymbolTable);
    }

    if(t->rule == "term factor") {
        genCode(t->children[0], topSymbolTable);
    }

    if(t->rule == "type INT") {

    }

}