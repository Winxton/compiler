// Starter code for CS241 assignments 8-11
//
// C++ translation by Simon Parent (Winter 2011),
// based on Java code by Ondrej Lhotak,
// which was based on Scheme code by Gord Cormack.
// Modified July 3, 2012 by Gareth Davies
// Modified July 2, 2014 by Brad Lushman
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <map>
using namespace std;

// procedure name - pair with signature and map of symbols
typedef pair<vector<string>, map<string, string> > SignatureSymbolTablePair;
typedef map<string, SignatureSymbolTablePair > TopSymbolTable;

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

// Data structure for storing the parse tree.
struct tree {
    string rule;
    vector<string> tokens;
    vector<tree*> children;
    ~tree() { for(int i=0; i<children.size(); i++) delete children[i]; }
};

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

pair<string, string> getNameAndType(tree *t) {
  // rule is "dcl type ID"
  pair<string, string> nameType;
  string name = t->children[1]->tokens[1];
  string type;

  // type could be INT or INT STAR, loop though and append them
  for(int idx=1/*skip the lhs*/; idx < t->children[0]->tokens.size(); ++idx) {
    type += t->children[0]->tokens[idx];
  }

  nameType.first = name;

  if (type == "INTSTAR") {
    type = "int*";
  } else { // type == "INT"
    type = "int";
  }

  nameType.second = type;

  return nameType;
}

void populateSignatures(tree *t, vector<string> &signature) {
  if (t->rule == "procedure INT ID LPAREN params RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE") 
  {
    populateSignatures(t->children[3], signature); // params
  }
  else if (t->rule == "main INT WAIN LPAREN dcl COMMA dcl RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE")
  {
    populateSignatures(t->children[3], signature); // dcl
    populateSignatures(t->children[5], signature); // dcl
  }
  else if (t->rule == "dcl type ID")
  {
    signature.push_back( getNameAndType(t).second ); // base case: append the type
  } 
  else 
  {
    for (vector<tree*>::iterator it = t->children.begin(); it != t->children.end(); it++) {
      populateSignatures(*it, signature);
    }
  }
}

string getProcedureName(tree *t) {
  if (t->rule == "procedure INT ID LPAREN params RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE") {
    return getProcedureName(t->children[1]); //ID
  } else { // t->rule == ID ..
    return t->tokens[1];
  }
}

void populateArgList(tree *t, vector<string> &argList) {

  if (t->rule == "arglist expr") {
    populateArgList(t->children[0], argList);
  } else if (t->rule == "arglist expr COMMA arglist") {
    populateArgList(t->children[0], argList);
    populateArgList(t->children[2], argList);
  } else if (t->rule == "expr term") {
    populateArgList(t->children[0], argList);
  } else if (t->rule == "term factor") {
    populateArgList(t->children[0], argList);
  } else if (t->rule == "factor ID" || t->rule == "factor NUM") {
    populateArgList(t->children[0], argList);
  } else if (t->tokens[0] == "ID") {
    argList.push_back(t->tokens[1]); // push argument name
  } else if (t->tokens[0] == "NUM") { // naked number
    argList.push_back("NUM");
  } else {
    throw string("ERROR: Could not parse Arguments: " + t->rule);
  }
}

bool wellTyped(tree *node) {
  return false;
}

string getType(const tree *node, TopSymbolTable &topSymbolTable, string currentProcedure) {
  if (node->tokens[0] == "ID") {
    return topSymbolTable[currentProcedure].second[node->tokens[1]];
  }

  if (node->tokens[0] == "NUM") {
    return "int";
  }

  if (node->tokens[0] == "NULL") {
    return "int*";
  }

  if (node->rule == "factor ID" || node->rule == "factor NUM" || node->rule == "factor NULL") {
    return getType(node->children[0], topSymbolTable, currentProcedure);
  }

  if (node->rule == "factor LPAREN expr RPAREN") {
    return getType(node->children[1], topSymbolTable, currentProcedure); // expr
  }

  if (node->rule == "factor AMP lvalue") {
    string LvalueType = getType(node->children[1], topSymbolTable, currentProcedure);
    if (LvalueType != "int") throw string("ERROR: & must be used on int");
    else return "int*";
  }
  if (node->rule == "factor NEW INT LBRACK expr RBRACK") {
    // The type of the derived expr must be int.
    string exprType = getType(node->children[3], topSymbolTable, currentProcedure);
    if (exprType != "int") 
      throw string("ERROR: new [] must use int as parameter, " + exprType + " given");

    return "int*";
  }
  if (node->rule == "factor ID LPAREN RPAREN") {
    //The procedure whose name is ID must have an empty signature.
    if (topSymbolTable[currentProcedure].first.size() != 0) 
      throw string("ERROR: Function takes in 0 parameters");

    return "int";
  }
  if (node->rule == "factor ID LPAREN arglist RPAREN") {
    // TODO need to check that all arguements are ints

    
    vector<string> argList;
    populateArgList(node->children[2], argList);
    string funcName = node->children[0]->tokens[1];
    vector<string> &functionSignature = topSymbolTable[funcName].first;

    cerr << "Inside: " << currentProcedure << ",  Function called: " << funcName << endl;
    cerr << argList.size() << " " << functionSignature.size() << endl;
    cerr << endl;

    if (argList.size() != functionSignature.size()) throw string("ERROR: Invalid number of arguments");

    for (int i =0; i<argList.size(); i++) {
        // type of parameter passed into the function (symbol is inside the current procedure)
        string varType = argList[i] == "NUM" ? "int" : topSymbolTable[currentProcedure].second[argList[i]];

        // type of the function signature
        string sigType = functionSignature[i];

        if (varType != sigType) {
            throw string("ERROR: Function expected " + sigType + ", got " + varType);
        }
    }

    return "int";
  }

  if (node->rule == "lvalue STAR factor") {
    string factorType = getType(node->children[1], topSymbolTable, currentProcedure);
    if (factorType == "int") throw string("TYPE ERROR: cannot deference an int");
    else return "int"; // factorType is int*
  }
  
  if (node->rule == "lvalue LPAREN lvalue RPAREN") {
    return getType(node->children[1], topSymbolTable, currentProcedure);
  }

  // singleton productions
  if (node->rule == "expr term" || node->rule == "term factor" || node->rule == "factor ID" || node->rule == "lvalue ID") {
    return getType(node->children[0], topSymbolTable, currentProcedure);
  }

  // The type of a term directly deriving anything other than just factor is int.
  if (node->tokens[0] == "term") {
    return "int";
  }

  if (node->rule == "expr expr PLUS term") {
    string L = getType(node->children[0], topSymbolTable, currentProcedure); // expr
    string R = getType(node->children[2], topSymbolTable, currentProcedure); // term
    if (L == "int" && R == "int") {
      return "int";
    } else if (L == "int*" && R == "int") {
      return "int*";
    } else if (L == "int" && R == "int*") {
      return "int*";
    } else {// (L == "int*" && R == "int*") {
      throw string("TYPE ERROR: cannot add two int*");
    }
  }

  if (node->rule == "expr expr MINUS term") {
    string L = getType(node->children[0], topSymbolTable, currentProcedure); // expr
    string R = getType(node->children[2], topSymbolTable, currentProcedure); // term
    if (L == "int" && R == "int") {
      return "int";
    } else if (L == "int*" && R == "int") {
      return "int*";
    } else if (L == "int" && R == "int*") {
      throw string("TYPE ERROR: cannot subtract int* from int");
    } else {// (L == "int*" && R == "int*") {
      return "int";
    }
  }

  // lhs of assignment
  if (node->rule == "dcl type ID") {
    return getType(node->children[1], topSymbolTable, currentProcedure);
  }

  throw string("ERROR: cannot be typed at " + node->rule);
}

void assertWellTyped(const tree *node, TopSymbolTable &topSymbolTable, string currentProcedure) {
  /*

  // declarations

  if (node->rule == "dcls dcls dcl BECOMES NUM SEMI") {
    if (getType(node->children[1], topSymbolTable, currentProcedure) != "int")
      throw string("ERROR: must assign number to int");
  }
  if (node->rule == "dcls dcls dcl BECOMES NULL SEMI") {
    if (getType(node->children[1], topSymbolTable, currentProcedure) != "int*")
      throw string("ERROR: must assign number to int");
  }

  // throw string("ERROR: cannot determine well typed at " + node->rule);
  */
}

// Compute symbols defined in t.
void genSymbols(tree *t, TopSymbolTable &topSymbolTable, string currentProcedure) {

  // rule is a declaration of a variable
  if (t->rule == "dcl type ID") 
  {
      pair<string, string> nameType = getNameAndType(t);

      // procedure with map already exists
      if (topSymbolTable.count(currentProcedure)) {
        
        // symbol already exists
        if (topSymbolTable[currentProcedure].second.count(nameType.first)) {
          throw string("ERROR: duplicate declaration of "+nameType.first + " in function: " + currentProcedure);
        } else {
          topSymbolTable[currentProcedure].second[nameType.first] = nameType.second;
        }
        
      // procedure does not exist
      } else {
        throw string("ERROR: something terrible happened!");
      }
  }
  // variable is used, check if declared before
  else if (t->rule == "factor ID" || t->rule == "lvalue ID") {
      // name of the id ( ID id_name )
      string idName = t->children[0]->tokens[1];
      // variable has not been declared
      if (topSymbolTable[currentProcedure].second.count(idName) == 0) {
        throw string("ERROR: variable not declared: " + idName);
      }
  }
  // procedure is used, check if declared before
  else if (t->rule == "factor ID LPAREN RPAREN" || t->rule == "factor ID LPAREN arglist RPAREN") {
    string procName = t->children[0]->tokens[1];
    // variable has not been declared
    if (topSymbolTable.count(procName) == 0) {
      throw string("ERROR: procedure not declared: " + procName);
    }
    // there exists a variable with the same name in the current scope
    if (topSymbolTable[currentProcedure].second.count(procName) != 0) {
      throw string("ERROR: variable not a function: " + procName);
    }
  }

  vector<string> signature;
  
  // was a procedure declared?
  bool procedureDeclared = false;
  
  if (t->rule == "procedures main") {
    currentProcedure = "wain";
    populateSignatures(t->children[0], signature); // pass main as root
    procedureDeclared = true;
  }
  else if (t->rule == "procedures procedure procedures") {
    currentProcedure = getProcedureName(t->children[0]); // pass procedure as root
    populateSignatures(t->children[0], signature); // pass procedure as root
    procedureDeclared = true;
  }

  // a new procedure
  if (procedureDeclared) 
  {

    // declaration of a procedure that ALREADY EXISTS - ERROR
    if (topSymbolTable.count(currentProcedure) && currentProcedure != "GLOBAL") {
      throw string("ERROR: Duplicate procedure declaration of " + currentProcedure);

    } else if (currentProcedure != "GLOBAL") {
        SignatureSymbolTablePair signatureSymbolTable;

        map<string, string> symbolTableMap;
        signatureSymbolTable.first = signature;
        signatureSymbolTable.second = symbolTableMap;

        topSymbolTable[currentProcedure] = signatureSymbolTable;
    }
  }


  for (vector<tree*>::iterator it = t->children.begin(); it != t->children.end(); it++) 
  {
    genSymbols(*it, topSymbolTable, currentProcedure);
  }

  // TYPECHECK
  if ( t->tokens[0] == "expr" || t->tokens[0] == "lvalue") {
    string type = getType(t, topSymbolTable, currentProcedure);
  } else {
    assertWellTyped(t, topSymbolTable, currentProcedure);
  }
  
}

void printSymbolTable(TopSymbolTable &topSymbolTable) {
  // print symbol table
  // iterate through procedures
  for (TopSymbolTable::iterator it = topSymbolTable.begin(); it!= topSymbolTable.end(); it++) {
    if (it != topSymbolTable.begin()) cerr << endl;

    string procedure = it->first;
    cerr << procedure;

    SignatureSymbolTablePair &sigSymTabPair = topSymbolTable[procedure];
    for (vector<string>::iterator it = sigSymTabPair.first.begin(); it != sigSymTabPair.first.end(); it++) {
      cerr << " " << *it;
    }
    cerr << endl;

    for (map<string, string>::iterator it2 = sigSymTabPair.second.begin(); 
      it2!= sigSymTabPair.second.end(); 
      it2++) {

      cerr << it2->first << " " << it2->second << endl;
    }
  }
}

// Generate the code for the parse tree t.
void genCode(tree *t) {
}

int main() {
  tree *parseTree = 0;
  // Main program.
  try {
    
    parseTree = readParse("start");
    
    //initialize the symbol table
    // procedure name -> name-type map for each procedure
    TopSymbolTable topSymbolTable;

    genSymbols(parseTree, topSymbolTable, "GLOBAL");
    printSymbolTable(topSymbolTable);

    genCode(parseTree);


  } catch(string msg) {
    cerr << msg << endl;
  }
  delete parseTree;
}
