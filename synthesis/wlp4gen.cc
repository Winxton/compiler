// Starter code for CS241 assignments 8-11
//
// C++ translation by Simon Parent (Winter 2011),
// based on Java code by Ondrej Lhotak,
// which was based on Scheme code by Gord Cormack.
// Modified July 3, 2012 by Gareth Davies
// Modified July 2, 2014 by Brad Lushman

#include "typechecker.h"
#include "codegen.h"
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <map>
using namespace std;

// The set of terminal symbols in the WLPP grammar.
const char * const terminals[] = {
  "BOF", "BECOMES", "COMMA", "ELSE", "EOF", "EQ", "GE", "GT", "ID", "IF",
  "INT", "LBRACE", "LE", "LPAREN", "LT", "MINUS", "NE", "NUM", "PCT", "PLUS",
  "PRINTLN", "RBRACE", "RETURN", "RPAREN", "SEMI", "SLASH", "STAR", "WAIN",
  "WHILE", "AMP", "LBRACK", "RBRACK", "NEW", "DELETE", "NULL"
};

const set<string> terms(terminals, terminals + sizeof(terminals)/sizeof(char*));

bool isTerminal(const string &sym) {
  return terms.count(sym);
}

// Read and return wlp4i parse tree.
tree *readParse(const string &lhs) {
  // Read a line from standard input.
  string line;
  getline(cin, line);
  if(!cin)
    throw string("ERROR: Unexpected end of file.");
  tree *ret = new tree();
  // Tokenize the line.
  istringstream ss(line);
  string token;
  while(ss >> token) {
    ret->tokens.push_back(token);
  }
  // Ensure that the rule is separated by single spaces.
  for(int idx=0; idx<ret->tokens.size(); ++idx) {
    if(idx>0) ret->rule += " ";
    ret->rule += ret->tokens[idx];
  }
  // Recurse if lhs is a nonterminal.
  if(!isTerminal(lhs)) {
    for(int idx=1/*skip the lhs*/; idx<ret->tokens.size(); ++idx) {
      ret->children.push_back(readParse(ret->tokens[idx]));
    }
  }
  return ret;
}

int main() {
  tree *parseTree = 0;
  // Main program.
  try {

    parseTree = readParse("start");

    // populate the symbol table and do precompile type check
    TopSymbolTable topSymbolTable;
    TypeChecker::genSymbols(parseTree, topSymbolTable, "GLOBAL");
    //TypeChecker::printSymbolTable(topSymbolTable);

    CodeGen::genCode(parseTree, topSymbolTable);

  } catch(string msg) {
    cerr << msg << endl;
  }
  delete parseTree;
}
