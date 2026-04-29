#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <map>

using namespace std;

class AutomatDFA {
protected:
    int numStates, numFinalStates;
    set <string> states;
    set <char> alphabet;
    string initialState;
    set <string> finalStates;
    map <string, map <char,string> > transitionTable;
public:
    AutomatDFA (int numStates, const set <string>& states, int numTransitions,
    const vector <string>& transitions, string  initialState, int numFinalStates,
    const set <string>& finalStates)
    : numStates(numStates),
      numFinalStates(numFinalStates),
      states(states),
      initialState(std::move(initialState)),
      finalStates(finalStates)
    {
        for (const string& currTransition : transitions) {
            stringstream ss(currTransition);
            string firstState, secondState;
            char currLetter;
            ss >> firstState >> secondState >> currLetter;
            alphabet.insert(currLetter);
            transitionTable[firstState][currLetter] = secondState;
        }
    }

    bool wordIsAccepted (const string& word) {
        string currState = initialState;
        for (int i = 0; i < word.length(); i++) {
            char currentLetter = word[i];
            if (transitionTable[currState].find(currentLetter) == transitionTable[currState].end())
                return false;
            currState = transitionTable[currState][currentLetter];
        }
        if (finalStates.find(currState) != finalStates.end())
            return true;
        return false;
    }

    virtual void printAlphabet() const {
        cout << "Alphabet: ";
        for (const char& letter : alphabet) {
            cout << letter << " ";
        }
        cout << endl;
    }

    virtual void printAcceptingPath(const string& word) const {
        string currState = initialState;

        for (int i = 0; i < word.length(); i++) {
            char currentLetter = word[i];

            if (!transitionTable.count(currState) || !transitionTable.at(currState).count(currentLetter)) {
                cout << "No accepting path found" << endl;
                return;
            }

            string nextState = transitionTable.at(currState).at(currentLetter);
            cout << currState << " --" << currentLetter << "--> " << nextState << endl;
            currState = nextState;
        }

        if (finalStates.find(currState) != finalStates.end()) {
            cout << "Accepted in state " << currState << endl << endl;
        }
        else {
            cout << "No accepting path found" << endl;
        }
    }
};

class AutomatNFA : public AutomatDFA {
    map <string, map <char, set <string>>> transitionTable;

    bool findAcceptingPath(const string& currentState, int index, const string& word,
                           vector<string>& path, string& finalState) const {
        if (index == word.length()) {
            if (finalStates.find(currentState) != finalStates.end()) {
                finalState = currentState;
                return true;
            }
            return false;
        }

        char currentLetter = word[index];

        if (!transitionTable.count(currentState) || !transitionTable.at(currentState).count(currentLetter)) {
            return false;
        }

        for (const string& nextState : transitionTable.at(currentState).at(currentLetter)) {
            string step = currentState + " --" + string(1, currentLetter) + "--> " + nextState;
            path.push_back(step);

            if (findAcceptingPath(nextState, index + 1, word, path, finalState)) {
                return true;
            }

            path.pop_back();
        }

        return false;
    }
public:
    AutomatNFA(int numStates, const set<string>& states, int numTransitions,
           const vector<string>& transitions, const string& initialState,
           int numFinalStates, const set<string>& finalStates)
    : AutomatDFA(numStates, states, numTransitions, transitions,
                 initialState, numFinalStates, finalStates)
    {
        for (const string& currTransition : transitions) {
            stringstream ss(currTransition);
            string firstState, secondState;
            char currLetter;

            ss >> firstState >> secondState >> currLetter;

            alphabet.insert(currLetter);
            transitionTable[firstState][currLetter].insert(secondState);
        }
    }

    virtual bool wordIsAccepted(const string& word) {
        set<string> currStates = {initialState};

        for (int i = 0; i < word.length(); i++) {
            char currentLetter = word[i];
            set<string> nextStates;
            for (const string& currState : currStates) {
                if (transitionTable[currState].count(currentLetter)) {
                    for (const string& secondState : transitionTable.at(currState).at(currentLetter)) {
                        nextStates.insert(secondState);
                    }
                }
            }
            currStates = nextStates;
            if (currStates.empty()) {
                return false;
            }
        }
        for (const string& endState : currStates) {
            if (finalStates.find(endState) != finalStates.end()) {
                return true;
            }
        }

        return false;
    }

    void printAcceptingPath(const string& word) const override {
        vector<string> path;
        string finalState;

        if (!findAcceptingPath(initialState, 0, word, path, finalState)) {
            cout << "No accepting path found" << endl;
            return;
        }

        if (path.empty()) {
            cout << "Accepted in state " << finalState << endl << endl;
            return;
        }

        for (const string& step : path) {
            cout << step << endl;
        }
        cout << "Accepted in state " << finalState << endl << endl;
    }
};

class AutomatLambdaNFA : public AutomatNFA {
    map <string, map <string, set <string>>> transitionTable;
    set <string> lambdaAlphabet;

    set<string> lambdaClosure(const set<string>& startStates) const {
        set<string> closure = startStates;
        vector<string> statesToProcess(startStates.begin(), startStates.end());

        while (!statesToProcess.empty()) {
            string currentState = statesToProcess.back();
            statesToProcess.pop_back();

            if (!transitionTable.count(currentState) || !transitionTable.at(currentState).count("lambda")) {
                continue;
            }

            for (const string& nextState : transitionTable.at(currentState).at("lambda")) {
                if (closure.insert(nextState).second) {
                    statesToProcess.push_back(nextState);
                }
            }
        }

        return closure;
    }

    bool findAcceptingPath(const string& currentState, int index, const string& word,
                           vector<string>& path, set<string>& statesInCurrentPath,
                           string& finalState) const {
        string currentConfiguration = currentState + "|" + to_string(index);

        if (statesInCurrentPath.find(currentConfiguration) != statesInCurrentPath.end()) {
            return false;
        }

        statesInCurrentPath.insert(currentConfiguration);

        if (index == word.length() && finalStates.find(currentState) != finalStates.end()) {
            finalState = currentState;
            statesInCurrentPath.erase(currentConfiguration);
            return true;
        }

        if (transitionTable.count(currentState) && transitionTable.at(currentState).count("lambda")) {
            for (const string& nextState : transitionTable.at(currentState).at("lambda")) {
                string step = currentState + " --lambda--> " + nextState;
                path.push_back(step);

                if (findAcceptingPath(nextState, index, word, path, statesInCurrentPath, finalState)) {
                    statesInCurrentPath.erase(currentConfiguration);
                    return true;
                }

                path.pop_back();
            }
        }

        if (index < word.length()) {
            string currentLetter(1, word[index]);

            if (transitionTable.count(currentState) && transitionTable.at(currentState).count(currentLetter)) {
                for (const string& nextState : transitionTable.at(currentState).at(currentLetter)) {
                    string step = currentState + " --" + currentLetter + "--> " + nextState;
                    path.push_back(step);

                    if (findAcceptingPath(nextState, index + 1, word, path, statesInCurrentPath, finalState)) {
                        statesInCurrentPath.erase(currentConfiguration);
                        return true;
                    }

                    path.pop_back();
                }
            }
        }

        statesInCurrentPath.erase(currentConfiguration);
        return false;
    }
public:
    AutomatLambdaNFA(int numStates, const set<string>& states, int numTransitions,
           const vector<string>& transitions, const string& initialState,
           int numFinalStates, const set<string>& finalStates)
    : AutomatNFA(numStates, states, 0, {},
                 initialState, numFinalStates, finalStates)
    {
        for (const string& currTransition : transitions) {
            stringstream ss(currTransition);
            string firstState, secondState;
            string currLetter;

            ss >> firstState >> secondState >> currLetter;

            if (currLetter != "lambda") {
                lambdaAlphabet.insert(currLetter);
            }
            transitionTable[firstState][currLetter].insert(secondState);
        }
    }

    bool wordIsAccepted(const string& word) override {
        set<string> currStates = lambdaClosure({initialState});

        for (char currentLetter : word) {
            string currLetter(1, currentLetter);
            set<string> nextStates;
            for (const string& currState : currStates) {
                if (transitionTable.count(currState) && transitionTable.at(currState).count(currLetter)) {
                    for (const string& secondState : transitionTable.at(currState).at(currLetter)) {
                        nextStates.insert(secondState);
                    }
                }
            }
            currStates = lambdaClosure(nextStates);
            if (currStates.empty()) {
                return false;
            }
        }
        for (const string& endState : currStates) {
            if (finalStates.find(endState) != finalStates.end()) {
                return true;
            }
        }

        return false;
    }

    void printAlphabet() const override {
        cout << "Alphabet: ";
        for (const string& letter : lambdaAlphabet) {
            cout << letter << " ";
        }
        cout << endl;
    }

    void printAcceptingPath(const string& word) const override {
        vector<string> path;
        set<string> statesInCurrentPath;
        string finalState;

        if (!findAcceptingPath(initialState, 0, word, path, statesInCurrentPath, finalState)) {
            cout << "No accepting path found" << endl;
            return;
        }

        if (path.empty()) {
            cout << "Accepted in state " << finalState << endl << endl;
            return;
        }

        for (const string& step : path) {
            cout << step << endl;
        }
        cout << "Accepted in state " << finalState << endl << endl;
    }
};

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

    fin >> numWords;
    for (int i=0; i<numWords; i++) {
        string currWord;
        fin >> currWord;
        if (LambdaNFA.wordIsAccepted(currWord)) {
            cout << "DA" << endl;
            LambdaNFA.printAcceptingPath(currWord);
        }
        else cout << "NU" << endl << endl;
    }

    return 0;
}
