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

string getType(const tree *node, TopSymbolTable &topSymbolTable, string currentProcedure);

void populateArgList(tree *t, TopSymbolTable &topSymbolTable, string currentProcedure, vector<string> &argList) {

  if (t->rule == "arglist expr") 
  {
    string exprType = getType(t->children[0], topSymbolTable, currentProcedure);
    argList.push_back(exprType);
  } 
  else if (t->rule == "arglist expr COMMA arglist") 
  {
    string exprType = getType(t->children[0], topSymbolTable, currentProcedure);
    argList.push_back(exprType);
    populateArgList(t->children[2], topSymbolTable, currentProcedure, argList);
  }
  else {
    throw string("ERROR: Could not parse Arguments: " + t->rule);
  }
}

// gets the type of a node, or throws error if incorrect
string getType(const tree *node, TopSymbolTable &topSymbolTable, string currentProcedure) {

  // Literals and identifiers

  if (node->tokens[0] == "ID") {
    return topSymbolTable[currentProcedure].second[node->tokens[1]];
  }

  if (node->tokens[0] == "NUM") {
    return "int";
  }

  if (node->tokens[0] == "NULL") {
    return "int*";
  }

  // singleton productions -> type of LHS is type of RHS
  if (node->rule == "expr term" 
    || node->rule == "term factor" 
    || node->rule == "factor ID" 
    || node->rule == "lvalue ID" 
    || node->rule == "factor ID" 
    || node->rule == "factor NUM" 
    || node->rule == "factor NULL")
  {
    return getType(node->children[0], topSymbolTable, currentProcedure);
  }

  // The type of a factor deriving LPAREN expr RPAREN is the same as the type of the expr.
  if (node->rule == "factor LPAREN expr RPAREN") {
    return getType(node->children[1], topSymbolTable, currentProcedure); // expr
  }

  // The type of a factor deriving AMP lvalue is int*. The type of the derived lvalue (i.e. the one preceded by AMP) must be int.
  if (node->rule == "factor AMP lvalue") {
    string LvalueType = getType(node->children[1], topSymbolTable, currentProcedure);
    if (LvalueType != "int") throw string("ERROR: & must be used on int");
    else return "int*";
  }

  // The type of a factor deriving NEW INT LBRACK expr RBRACK is int*. The type of the derived expr must be int.
  if (node->rule == "factor NEW INT LBRACK expr RBRACK") {
    // The type of the derived expr must be int.
    string exprType = getType(node->children[3], topSymbolTable, currentProcedure);
    
    if (exprType != "int") 
      throw string("ERROR: new [] must use int as parameter, " + exprType + " given");

    return "int*";
  }

  // function call with no arguments
  // The type of a factor deriving ID LPAREN RPAREN is int.
  // The procedure whose name is ID must have an empty signature.
  if (node->rule == "factor ID LPAREN RPAREN") {
    string funcName = node->children[0]->tokens[1];

    if (topSymbolTable[funcName].first.size() != 0) 
      throw string("ERROR: Function takes in 0 parameters");

    return "int";
  }

  // function call with arguments
  // The type of a factor deriving ID LPAREN arglist RPAREN is int. 
  // The procedure whose name is ID must have a signature whose length is equal to the number of expr strings 
  // (separated by COMMA) that are derived from arglist. 
  // Further the types of these expr strings must exactly match, 
  // in order, the types in the procedure's signature.
  if (node->rule == "factor ID LPAREN arglist RPAREN") {

    vector<string> argTypeList;
    populateArgList(node->children[2], topSymbolTable, currentProcedure, argTypeList);
    string funcName = node->children[0]->tokens[1];
    vector<string> &functionSignature = topSymbolTable[funcName].first;

    if (argTypeList.size() != functionSignature.size()) throw string("ERROR: Invalid number of arguments");

    for (int i =0; i<argTypeList.size(); i++) {
        // type of parameter passed into the function (symbol is inside the current procedure)
        string varType = argTypeList[i];

        // type of the function signature
        string sigType = functionSignature[i];

        if (varType != sigType) {
            throw string("ERROR: Function expected " + sigType + ", got " + varType);
        }
    }

    return "int";
  }

  // The type of a factor or lvalue deriving STAR factor is int. 
  // The type of the derived factor (i.e. the one preceded by STAR) must be int*.
  if (node->rule == "lvalue STAR factor" || node->rule == "factor STAR factor") 
  {
    string factorType = getType(node->children[1], topSymbolTable, currentProcedure);

    if (factorType == "int") {
      throw string("TYPE ERROR: cannot deference an int");
    } 
    else {
      return "int"; // factorType is int*
    }
  }
  
  // The type of an lvalue deriving LPAREN lvalue RPAREN is the same as the type of the derived lvalue.
  if (node->rule == "lvalue LPAREN lvalue RPAREN") {
    return getType(node->children[1], topSymbolTable, currentProcedure);
  }

  // The type of a term directly deriving anything other than just factor is int.
  // The term and factor directly derived from such a term must have type int.
  if (node->rule == "term term STAR factor" || node->rule == "term term SLASH factor" || node->rule == "term term PCT factor") 
  {
    if (getType(node->children[0], topSymbolTable, currentProcedure) != "int"
      || getType(node->children[2], topSymbolTable, currentProcedure) != "int") 
      {
        string op = node->children[1]->tokens[1];
        throw string("ERROR: invalid operation: " + op + " cannot be used with int* " + " in " + currentProcedure);
      }

    return "int";
  }

  /*
  When expr derives expr PLUS term:
  The derived expr and the derived term may both have type int, in which case the type of the expr deriving them is int.
  The derived expr may have type int* and the derived term may have type int, in which case the type of the expr deriving them is int*.
  The derived expr may have type int and the derived term may have type int*, in which case the type of the expr deriving them is int*.
  */
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
  /*
  When expr derives expr MINUS term:
  The derived expr and the derived term may both have type int, in which case the type of the expr deriving them is int.
  The derived expr may have type int* and the derived term may have type int, in which case the type of the expr deriving them is int*.
  The derived expr and the derived term may both have type int*, in which case the type of the expr deriving them is int.
  */
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
  
  if (node->rule == "dcl type ID") {
    return getType(node->children[1], topSymbolTable, currentProcedure);
  }

  throw string("ERROR: cannot be typed at " + node->rule);
}

// assert well typed for expressions with no type (statements and tests)
// These are program elements that do not themselves have types, 
// but demand that some of their subelements be given particular types
void assertWellTyped(const tree *node, TopSymbolTable &topSymbolTable, string currentProcedure) {
  /*
  // RULES 
  The second dcl in the sequence directly derived from procedure must derive a type that derives INT.
  The expr in the sequence directly derived from procedure must have type int.
  When statement derives lvalue BECOMES expr SEMI, the derived lvalue and the derived expr must have the same type.
  When statement derives PRINTLN LPAREN expr RPAREN SEMI, the derived expr must have type int.
  When statement derives DELETE LBRACK RBRACK expr SEMI, the derived expr must have type int*.
  Whenever test directly derives a sequence containing two exprs, they must both have the same type.
  When dcls derives dcls dcl BECOMES NUM SEMI, the derived dcl must derive a sequence containing a type that derives INT.
  When dcls derives dcls dcl BECOMES NULL SEMI, the derived dcl must derive a sequence containing a type that derives INT STAR.
  Semantics
  */
  
  /*
  This only needs to check for those where getType() is used for comparison

  for example, statement → WHILE LPAREN test RPAREN LBRACE statements RBRACE 
  does not need to be checked because "test" and "statements" are 
  checked in the next recursive call from genSymbols

  This considers all possibilities because getType(...) asserts the correctness
  of lvalue and expr, which can only come from statements and tests
  */

  // declarations

  if (node->rule == "dcls dcls dcl BECOMES NUM SEMI") {
    if (getType(node->children[1], topSymbolTable, currentProcedure) != "int") {
      throw string("ERROR: must assign number to int, NUM given");
    }
  }

  if (node->rule == "dcls dcls dcl BECOMES NULL SEMI") {
    if (getType(node->children[1], topSymbolTable, currentProcedure) != "int*") {
      throw string("ERROR: must assign number to int, NULL given");
    }
  }

  // a comparison is well typed if its arguments have the same type
  if (node->rule == "test expr EQ expr"
    || node->rule == "test expr NE expr"
    || node->rule == "test expr LT expr"
    || node->rule == "test expr LE expr"
    || node->rule == "test expr GE expr"
    || node->rule == "test expr GT expr") 
  {
      // get type does not return error ->  well typed
      string L = getType(node->children[0], topSymbolTable, currentProcedure);
      string R = getType(node->children[2], topSymbolTable, currentProcedure);
      if (L != R) {
        throw string("ERROR: comparison between " + L + " and " + R);
      }
  }

  if (node->rule == "main INT WAIN LPAREN dcl COMMA dcl RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE") {
    currentProcedure = "wain";

    string second = getType(node->children[5], topSymbolTable, currentProcedure); // second parameter in wain
    if (second != "int") throw string("ERROR: wain must take int as second argument");

    //assertWellTyped(node->children[9], topSymbolTable, currentProcedure); // statements

    string returnValue = getType(node->children[11], topSymbolTable, currentProcedure); // return value
    if (returnValue != "int")  throw string("ERROR: wain must return int");
  }

  if (node->rule == "procedure INT ID LPAREN params RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE") {
    currentProcedure = node->children[1]->tokens[1];

    //assertWellTyped(node->children[7], topSymbolTable, currentProcedure); // statements

    string returnValue = getType(node->children[9], topSymbolTable, currentProcedure); // return value (expr)
    if (returnValue != "int"){
      string procedureName = node->children[1]->tokens[1];
      throw string("ERROR: procedure" + procedureName + " must return int");
    }
  }

  if (node->rule == "statement lvalue BECOMES expr SEMI") {
    // get type does not return error ->  well typed
    string L = getType(node->children[0], topSymbolTable, currentProcedure);
    string R = getType(node->children[2], topSymbolTable, currentProcedure);
    if (L != R) {
      throw string("ERROR: cannot assign " + R + " to variable of type " + L);
    }
  }

  if (node->rule == "statement PRINTLN LPAREN expr RPAREN SEMI") 
  {
    string exprType = getType(node->children[2], topSymbolTable, currentProcedure); // expr
    if (exprType != "int") {
      throw string("ERROR: println must use int as parameter, " + exprType + " given");
    }
  }

  if (node->rule == "statement DELETE LBRACK RBRACK expr SEMI") {
    string exprType = getType(node->children[3], topSymbolTable, currentProcedure); // expr
    if (exprType != "int*") {
      throw string("ERROR: delete must use int* as parameter, " + exprType + " given");
    }
  }
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
  assertWellTyped(t, topSymbolTable, currentProcedure);
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
