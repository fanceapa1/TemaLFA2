#ifndef AUTOMAT_LAMBDA_NFA_H
#define AUTOMAT_LAMBDA_NFA_H

#include "AutomatNFA.h"

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

    AutomatDFA transformToDFA() const override {
        return this->transformToNFA().transformToDFA().minimizeDFA();
    }
};

#endif
