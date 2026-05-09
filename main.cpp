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
}
