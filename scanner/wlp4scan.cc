#include "kind.h"
#include "lexer.h"
#include <iostream>
using namespace std;
using ASM::Token;
using ASM::Lexer;

int main(int argc, char* argv[]) {
    vector< vector<Token*> > tokLines;
    try {
        Lexer lexer;
        string line;
        while (getline(cin, line)) {
            tokLines.push_back(lexer.scan(line));
        }

        vector<vector<Token*> >::iterator it;
        for(it = tokLines.begin(); it != tokLines.end(); ++it)
        {
            vector<Token*>::iterator it2;

            for(it2 = it->begin(); it2 != it->end(); ++it2)
            {
                cout << (*it2)->toString() << " " << (*it2)->getLexeme() << endl;
            }
        }

    } catch(const string& msg){
        // If an exception occurs print the message and end the program
        cerr << msg << endl;
    }
}
