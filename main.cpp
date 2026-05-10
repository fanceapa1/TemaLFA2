/*
#include <fstream>
#include <iostream>
#include <limits>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "AutomatPDA.h"

using namespace std;

string trim(const string& text) {
    size_t firstCharacter = text.find_first_not_of(" \t\r\n");
    if (firstCharacter == string::npos) {
        return "";
    }

    size_t lastCharacter = text.find_last_not_of(" \t\r\n");
    return text.substr(firstCharacter, lastCharacter - firstCharacter + 1);
}

set<string> parseSymbolSet(const string& line) {
    set<string> symbols;
    string symbol;
    stringstream ss(line);

    while (ss >> symbol) {
        if (symbol != "-") {
            symbols.insert(symbol);
        }
    }

    return symbols;
}

int main() {
    ifstream fin("../input.txt");

    string statesLine, inputAlphabetLine, stackAlphabetLine;
    getline(fin, statesLine);
    getline(fin, inputAlphabetLine);
    getline(fin, stackAlphabetLine);

    set<string> states = parseSymbolSet(statesLine);
    set<string> inputAlphabet = parseSymbolSet(inputAlphabetLine);
    set<string> stackAlphabet = parseSymbolSet(stackAlphabetLine);

    int numTransitions = 0;
    fin >> numTransitions;
    fin.ignore(numeric_limits<streamsize>::max(), '\n');

    vector<string> transitions;
    for (int i = 0; i < numTransitions; i++) {
        string transitionLine;
        getline(fin, transitionLine);
        transitions.push_back(transitionLine);
    }

    string initialState, initialStackSymbol, finalStatesLine, acceptingMethod, word;
    getline(fin, initialState);
    getline(fin, initialStackSymbol);
    getline(fin, finalStatesLine);
    fin >> acceptingMethod;
    fin >> word;

    set<string> finalStates = parseSymbolSet(finalStatesLine);

    AutomatPDA PDA(states, inputAlphabet, stackAlphabet, transitions,
                   trim(initialState), trim(initialStackSymbol),
                   finalStates, acceptingMethod);

    if (PDA.wordIsAccepted(word)) {
        cout << "ACCEPTAT";
    }
    else {
        cout << "RESPINS";
    }

    return 0;
} */



#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "AutomatLambdaNFA.h"

using namespace std;

int main() {
    ifstream fin("../input.txt");

    int numStates = 0, numFinalStates = 0, numTransitions = 0, numWords = 0;
    set <string> states, finalStates;
    vector <string> transitions;
    string initialState;

    fin >> numStates;
    for (int i=0; i<numStates; i++) {
        string currState;
        fin >> currState;
        states.insert(currState);
    }

    fin >> numTransitions;
    fin >> ws;
    for (int i=0; i<numTransitions; i++) {
        string currTransition;
        getline(fin, currTransition);
        transitions.push_back(currTransition);
    }
    fin >> initialState;
    fin >> numFinalStates;

    for (int i=0; i<numFinalStates; i++) {
        string currFinalState;
        fin>>currFinalState;
        finalStates.insert(currFinalState);
    }

    AutomatLambdaNFA LambdaNFA(numStates, states, numTransitions, transitions, initialState, numFinalStates, finalStates);
    LambdaNFA.printAlphabet();
    AutomatDFA DFA = LambdaNFA.transformToDFA();

    fin >> numWords;
    for (int i=0; i<numWords; i++) {
        string currWord;
        fin >> currWord;
        if (DFA.wordIsAccepted(currWord)) {
            cout << "DA" << endl;
            DFA.printAcceptingPath(currWord);
        }
        else cout << "NU" << endl << endl;
    }
    cout << LambdaNFA.toRegularExpression();
    return 0;
}