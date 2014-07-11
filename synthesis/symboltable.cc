#include "symboltable.h"
#include <iostream>
#include <cstdlib>
using namespace std;

SymbolTable *SymbolTable::instance = 0;

SymbolTable *SymbolTable::getInstance() {
	if (!instance) {
		instance = new SymbolTable();
		atexit(cleanup);
	}
	return instance;
}

void SymbolTable::cleanup() {
    delete instance;
}

int SymbolTable::getNumSymbolsAtProcedure(string procedure) {
	return symbolTable.count(procedure);
}

bool SymbolTable::hasProcedure(string procedure) {
	return getNumSymbolsAtProcedure(procedure) > 0;
}

bool SymbolTable::hasSymbolAtProcedure(string procedure, string symbol) {
	return symbolTable[procedure].second.count(symbol) != 0;
}

void SymbolTable::setSymbolType(string procedure, string symbolName, string symbolValue) {
	symbolTable[procedure].second[symbolName].first = symbolValue;
}

void SymbolTable::setSymbolOffset(std::string procedure, std::string symbolName, int offset) {
	symbolTable[procedure].second[symbolName].second = offset;
}

void SymbolTable::setProcedure(string procedure, SignatureInnerMapPair innerPair) {
	symbolTable[procedure] = innerPair;
}

string SymbolTable::getType(std::string procedure, std::string symbolName) {
	return symbolTable[procedure].second[symbolName].first;
}

int SymbolTable::getOffset(std::string procedure, std::string symbolName) {
	return symbolTable[procedure].second[symbolName].second;
}

std::vector<std::string> &SymbolTable::getSignature(std::string procedure) {
	return symbolTable[procedure].first;
}

void SymbolTable::print() {
  cerr << "--- SYMBOL TABLE ----" << endl;
  // print symbol table
  // iterate through procedures
  for (ProcedureTable::iterator it = symbolTable.begin(); it!= symbolTable.end(); it++) {
    if (it != symbolTable.begin()) cerr << endl;

    string procedure = it->first;
    cerr << procedure;

    SignatureInnerMapPair &sigSymTabPair = symbolTable[procedure];
    for (vector<string>::iterator it = sigSymTabPair.first.begin(); it != sigSymTabPair.first.end(); it++) {
      cerr << " " << *it;
    }
    cerr << endl;

    for (InnerSymbolMap::iterator it2 = sigSymTabPair.second.begin(); it2!= sigSymTabPair.second.end(); it2++) {

      cerr << it2->first << " " << it2->second.first << endl;
    }
  }
  cerr << "--- END SYMBOL TABLE ----" << endl;
  cerr << endl << endl;
}
