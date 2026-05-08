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
    virtual ~AutomatDFA() = default;

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

    virtual bool wordIsAccepted (const string& word) {
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

    bool wordIsAccepted(const string& word) override {
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

    AutomatDFA transformToDFA() const {
        vector<set<string>> subsets(1);

        for (const string& state : states) {
            int currentSize = subsets.size();

            for (int i = 0; i < currentSize; i++) {
                set<string> newSubset = subsets[i];
                newSubset.insert(state);
                subsets.push_back(newSubset);
            }
        }

        auto getSubsetName = [](const set<string>& subset) {
            if (subset.empty()) {
                return string("{}");
            }

            string subsetName = "{";
            bool isFirstState = true;

            for (const string& state : subset) {
                if (!isFirstState) {
                    subsetName += ",";
                }

                subsetName += state;
                isFirstState = false;
            }

            subsetName += "}";
            return subsetName;
        };

        set<string> dfaStates;
        set<string> dfaFinalStates;
        vector<string> dfaTransitions;

        for (const set<string>& subset : subsets) {
            string subsetName = getSubsetName(subset);
            dfaStates.insert(subsetName);

            for (const string& state : subset) {
                if (finalStates.count(state)) {
                    dfaFinalStates.insert(subsetName);
                    break;
                }
            }
        }

        for (const set<string>& subset : subsets) {
            string fromState = getSubsetName(subset);

            for (char letter : alphabet) {
                set<string> nextSubset;

                for (const string& state : subset) {
                    if (transitionTable.count(state) && transitionTable.at(state).count(letter)) {
                        for (const string& nextState : transitionTable.at(state).at(letter)) {
                            nextSubset.insert(nextState);
                        }
                    }
                }

                string toState = getSubsetName(nextSubset);
                dfaTransitions.push_back(fromState + " " + toState + " " + string(1, letter));
            }
        }

        string dfaInitialState = getSubsetName({initialState});

        return AutomatDFA(dfaStates.size(), dfaStates,
                          dfaTransitions.size(), dfaTransitions,
                          dfaInitialState, dfaFinalStates.size(), dfaFinalStates);
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

    AutomatNFA transformToNFA() const {
        map<string, map<string, set<string>>> completedTransitions = transitionTable;

        bool addedNewTransition;
        do {
            addedNewTransition = false;

            for (const string& p : states) {
                for (const string& q : states) {
                    for (const string& r : states) {
                        if (completedTransitions[p]["lambda"].count(q) &&
                            completedTransitions[q]["lambda"].count(r)) {
                            if (completedTransitions[p]["lambda"].insert(r).second) {
                                addedNewTransition = true;
                            }
                        }
                    }
                }
            }
        } while (addedNewTransition);

        set<string> newFinalStates = finalStates;
        for (const string& p : states) {
            for (const string& finalState : finalStates) {
                if (completedTransitions[p]["lambda"].count(finalState)) {
                    newFinalStates.insert(p);
                }
            }
        }

        map<string, map<char, set<string>>> nfaTransitionTable;

        for (const auto& stateTransitions : completedTransitions) {
            const string& fromState = stateTransitions.first;

            for (const auto& letterTransitions : stateTransitions.second) {
                const string& letter = letterTransitions.first;

                if (letter == "lambda") {
                    continue;
                }

                for (const string& toState : letterTransitions.second) {
                    nfaTransitionTable[fromState][letter[0]].insert(toState);
                }
            }
        }

        for (const string& p : states) {
            for (const string& q : completedTransitions[p]["lambda"]) {
                if (!completedTransitions.count(q)) {
                    continue;
                }

                for (const auto& letterTransitions : completedTransitions[q]) {
                    const string& letter = letterTransitions.first;

                    if (letter == "lambda") {
                        continue;
                    }

                    for (const string& r : letterTransitions.second) {
                        nfaTransitionTable[p][letter[0]].insert(r);
                    }
                }
            }
        }

        vector<string> nfaTransitions;
        for (const auto& stateTransitions : nfaTransitionTable) {
            const string& fromState = stateTransitions.first;

            for (const auto& letterTransitions : stateTransitions.second) {
                char letter = letterTransitions.first;

                for (const string& toState : letterTransitions.second) {
                    nfaTransitions.push_back(fromState + " " + toState + " " + string(1, letter));
                }
            }
        }

        return AutomatNFA(numStates, states, nfaTransitions.size(),
                          nfaTransitions, initialState,
                          newFinalStates.size(), newFinalStates);
    }

    AutomatDFA transformToDFA() const {
        return this->transformToNFA().transformToDFA();
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
