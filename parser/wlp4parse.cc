#include <set>
#include <string>
#include <iostream>
#include <fstream>
#include <list>
#include <sstream>
#include <stack>
#include <map>
#include <vector>
using namespace std;

set<string> terms;
set<string> nonterms;
vector<string> prods;
string start;

struct Tree {
    string rule;
    list<Tree*> children;

    ~Tree() {
        for(list<Tree*>::iterator it=children.begin(); it != children.end(); it++) {  // delete all subtrees
            delete (*it);
        }
    }
};

void readarr(istream &in, vector<string> &t) {
    int n;
    string temp;
    getline(in,temp);
    istringstream iss(temp);
    iss >> n;
    for(int i = 0; i < n; i++) {
        getline(in,temp);
        t.push_back(temp);
    }
}

void readsyms(istream &in, set<string> &t) {
    int n;
    string temp;
    getline(in,temp);
    istringstream iss(temp);
    iss >> n;
    for(int i = 0; i < n; i++) {
        getline(in,temp);
        t.insert(temp);
    }
}

void traverse(Tree *t, int d) {
    //for(int i = 0; i < d; i++) cout << " ";
    cout << t->rule << endl; // print root
    for(list<Tree*>::iterator it=(t->children).begin(); it != (t->children).end(); it++) {  // print all subtrees
        traverse(*it, d+1);
    }
}

void popper(stack<Tree *> &myStack, list<string> &rhs, string rule) {
    Tree *n = new Tree();
    n->rule = rule;
    for(list<string>::iterator it=rhs.begin(); it != rhs.end(); it++){
        Tree *tmp = myStack.top();
        n->children.push_front(tmp);
        myStack.pop();
    }
    myStack.push(n);
}

void addToParseTree(stack<Tree*> &parseTree, string &prod, bool terminal) {
    // update parsetree
    string l;
    list<string> r; // rhs symbols

    istringstream rulestr(prod);

    // only add to RHS if not terminal
    // if non-terminal, add the entire thing
    if (!terminal) 
    {
        rulestr >> l; // lhs symbol
        string s;

        while(rulestr >> s) {
            r.push_back(s); // include both terminal and non-terminal
        }
    }
    popper(parseTree, r, prod); // reduce rule
}

void populateLookupTable(istream &in, vector< map<string, pair<string, int> >* > &tbl) 
{
    int numStates;
    in >> numStates;

    for (int i =0; i<numStates; i++) {
        tbl.push_back( new map<string, pair<string, int> >() );
    }

    int numTransitions;
    in >> numTransitions;

    // populate lookup table with mappings    
    for (int i =0; i< numTransitions; i++) {
        int startState;
        in >> startState;
        
        string input;
        in >> input;

        pair<string, int> p;
        in >> p.first; // action
        in >> p.second; // next state

        (*tbl[startState])[input] = p;
    }
}

string intToString(int n) {
    stringstream ss;
    ss << n;
    return ss.str();
}

int stringToInt(string s) {
    istringstream iss(s);
    int n;
    iss >> n;
    return n;
}

int shift(stack<Tree*> &parseTree, stack<string> &myStack, string &input, int nextState) {

    myStack.push(input);
    myStack.push(intToString(nextState));

    addToParseTree(parseTree, input, true);

    return nextState;
}

string getlhs(string &prod) {
    istringstream iss(prod);
    string lhs;
    iss >> lhs;
    return lhs;
}

int reduce(stack<Tree*> &parseTree, stack<string> &myStack, string &prod,
    vector< map<string, pair<string, int> >* > &lookupTbl) {

    istringstream iss(prod);
    string lhs;
    iss >> lhs;

    string temp;
    // backtrack RHS
    while (iss >> temp) {
        myStack.pop();
        myStack.pop();
    }

    // push LHS
    int curState = stringToInt(myStack.top());
    pair<string, int> actionState = (*lookupTbl[curState])[lhs];

    myStack.push(lhs);
    myStack.push(intToString(actionState.second));

    addToParseTree(parseTree, prod, false);

    return actionState.second;
}

void printStack(vector<string> &myStack) {
    for(vector<string>::iterator it=myStack.begin(); it != myStack.end(); it++) {
        cout << *it << " | ";
    }
}

Tree* parse(vector< map<string, pair<string, int> >* > &lookupTbl) {
    stack<Tree*> parseTree;
    stack<string> myStack; // stack containing history

    myStack.push("0"); // push first state
    int curState = 0;

    // list of unread input (both type and lexeme) (maybe use pair?)
    list<string> unread; 

    string kind;
    string lexeme;

    while (true) {
        cin >> kind;
        cin >> lexeme;
        if (cin.eof()) break;
        unread.push_back(kind);
        unread.push_back(lexeme);
    }

    //append BOF and EOF
    unread.push_front("BOF");
    unread.push_front("BOF");
    unread.push_back("EOF");
    unread.push_back("EOF");

    int index = 0; // does not include BOF
    string production = "";

    while (!unread.empty()) {
        kind = unread.front();
        list<string>::iterator secondPtr = ++ unread.begin();
        lexeme = *(secondPtr);

        if ((*lookupTbl[curState]).count(kind) > 0) 
        {
            // pair of <action, nextstate>
            pair<string, int> actionState = (*lookupTbl[curState])[kind];

            // progress to next state if shift
            if (actionState.first == "shift") {

                string terminalString = kind + " " + lexeme;
                curState = shift(parseTree, myStack, terminalString, actionState.second);
                unread.pop_front();
                unread.pop_front();
                index ++;

                // if kind is EOF, accept
                if (kind == "EOF") {
                    // look for production which includes S
                    for (vector<string>::iterator it = prods.begin(); it != prods.end(); it++) {
                        if ( (*it).compare(0, start.length(), start) == 0) {
                            addToParseTree(parseTree, *it, false);
                            break;
                        }
                    } 
                }
            }
            // reduce -> backtrack num items in RHS and push LHS
            else { // (actionState.first == "reduce")
                // rule to be used
                production = prods[actionState.second];
                curState = reduce(parseTree, myStack,
                    prods[actionState.second],
                    lookupTbl);
            }

        } else {
            throw string("ERROR at " + intToString(index));
        }
    }

    string lhs; // lhs symbol

    return parseTree.top();
}


int main(){
    // use the WLP4 Specification file 
    ifstream wlp4spec("WLP4.lr1");
    readsyms(wlp4spec, terms); // read terminals
    readsyms(wlp4spec, nonterms); // read nonterminals
    getline(wlp4spec,start); // read start symbol
    readarr(wlp4spec, prods); // read production rules
    
    // create lookup for action given current state and input
    // m: start_state, input -> (pair) action, next_state
    vector< map<string, pair<string, int> >* > lookupTbl;
    populateLookupTable(wlp4spec, lookupTbl);

    try {

        Tree* parseTree = parse(lookupTbl);
        traverse(parseTree, 0);
        delete parseTree;

    } catch (string &err) {
        cerr << err << endl;
    }
    
    vector< map<string, pair<string, int> >* >::iterator it;
    for(it = lookupTbl.begin(); it != lookupTbl.end(); ++it){
        delete *it;
    }
}
