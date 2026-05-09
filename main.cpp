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

    return 0;
}
