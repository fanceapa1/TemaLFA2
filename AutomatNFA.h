#ifndef AUTOMAT_NFA_H
#define AUTOMAT_NFA_H

#include "AutomatDFA.h"

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

    virtual AutomatDFA transformToDFA() const {
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

#endif
