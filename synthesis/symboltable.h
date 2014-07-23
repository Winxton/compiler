#ifndef __SYMBOLTABLE_H__
#define __SYMBOLTABLE_H__

#include <string>
#include <vector>
#include <map>

// Data structure for storing the parse tree.
struct tree {
    std::string rule;
    std::string type;
    std::vector<std::string> tokens;
    std::vector<tree*> children;
    ~tree() { for(int i=0; i<children.size(); i++) delete children[i]; }
};

// MAP: procedure name -> pair(signature , map <symbol, pair(type, offset) >
typedef std::pair<std::string, int> TypeOffsetPair;
typedef std::map<std::string, TypeOffsetPair > InnerSymbolMap;
typedef std::pair<std::vector<std::string>, InnerSymbolMap> SignatureInnerMapPair;
typedef std::map<std::string, SignatureInnerMapPair > ProcedureTable;

class SymbolTable {
	static SymbolTable *instance;
public:
        ProcedureTable symbolTable;
	static SymbolTable *getInstance();
	static void cleanup();

	bool hasProcedure(std::string procedure);
	int getNumSymbolsAtProcedure(std::string procedure);
	bool hasSymbolAtProcedure(std::string procedure, std::string symbol);

	void setSymbolType(std::string procedure, std::string symbolName, std::string symbolValue);
	void setSymbolOffset(std::string procedure, std::string symbolName, int offset);
	void setProcedure(std::string procedure, SignatureInnerMapPair innerPair);

	std::vector<std::string> &getSignature(std::string procedure);
        InnerSymbolMap &getInnerSymbolMap(std::string procedure);
	std::string getType(std::string procedure, std::string symbolName);
	int getOffset(std::string procedure, std::string symbolName);

	void print();
};

#endif
