#ifndef AUTOMAT_DFA_H
#define AUTOMAT_DFA_H

#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

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

    AutomatDFA minimizeDFA() const {
        set<string> reachableStates;
        vector<string> statesToProcess;

        if (states.count(initialState)) {
            reachableStates.insert(initialState);
            statesToProcess.push_back(initialState);
        }

        for (int i = 0; i < statesToProcess.size(); i++) {
            const string& currentState = statesToProcess[i];

            if (!transitionTable.count(currentState)) {
                continue;
            }

            for (const auto& transition : transitionTable.at(currentState)) {
                const string& nextState = transition.second;

                if (states.count(nextState) && reachableStates.insert(nextState).second) {
                    statesToProcess.push_back(nextState);
                }
            }
        }

        set<string> reachableFinalStates;
        set<string> reachableNonFinalStates;

        for (const string& state : reachableStates) {
            if (finalStates.count(state)) {
                reachableFinalStates.insert(state);
            }
            else {
                reachableNonFinalStates.insert(state);
            }
        }

        vector<set<string>> partitions;
        if (!reachableFinalStates.empty()) {
            partitions.push_back(reachableFinalStates);
        }
        if (!reachableNonFinalStates.empty()) {
            partitions.push_back(reachableNonFinalStates);
        }

        bool partitionChanged;
        do {
            map<string, int> stateToPartition;
            for (int i = 0; i < partitions.size(); i++) {
                for (const string& state : partitions[i]) {
                    stateToPartition[state] = i;
                }
            }

            vector<set<string>> refinedPartitions;

            for (const set<string>& partition : partitions) {
                map<vector<int>, set<string>> statesBySignature;

                for (const string& state : partition) {
                    vector<int> signature;

                    for (char letter : alphabet) {
                        int destinationPartition = -1;

                        if (transitionTable.count(state) && transitionTable.at(state).count(letter)) {
                            const string& destinationState = transitionTable.at(state).at(letter);

                            if (stateToPartition.count(destinationState)) {
                                destinationPartition = stateToPartition[destinationState];
                            }
                        }

                        signature.push_back(destinationPartition);
                    }

                    statesBySignature[signature].insert(state);
                }

                for (const auto& group : statesBySignature) {
                    refinedPartitions.push_back(group.second);
                }
            }

            partitionChanged = (refinedPartitions != partitions);
            partitions = refinedPartitions;
        } while (partitionChanged);

        map<string, int> stateToPartition;
        for (int i = 0; i < partitions.size(); i++) {
            for (const string& state : partitions[i]) {
                stateToPartition[state] = i;
            }
        }

        auto getPartitionName = [](const set<string>& partition) {
            if (partition.size() == 1) {
                return *partition.begin();
            }

            string partitionName = "{";
            bool isFirstState = true;

            for (const string& state : partition) {
                if (!isFirstState) {
                    partitionName += ",";
                }

                partitionName += state;
                isFirstState = false;
            }

            partitionName += "}";
            return partitionName;
        };

        set<string> minimizedStates;
        set<string> minimizedFinalStates;
        vector<string> minimizedTransitions;

        for (const set<string>& partition : partitions) {
            string partitionName = getPartitionName(partition);
            minimizedStates.insert(partitionName);

            for (const string& state : partition) {
                if (finalStates.count(state)) {
                    minimizedFinalStates.insert(partitionName);
                    break;
                }
            }
        }

        for (int i = 0; i < partitions.size(); i++) {
            const string& representativeState = *partitions[i].begin();
            string fromState = getPartitionName(partitions[i]);

            if (!transitionTable.count(representativeState)) {
                continue;
            }

            for (char letter : alphabet) {
                if (!transitionTable.at(representativeState).count(letter)) {
                    continue;
                }

                const string& destinationState = transitionTable.at(representativeState).at(letter);

                if (!stateToPartition.count(destinationState)) {
                    continue;
                }

                string toState = getPartitionName(partitions[stateToPartition[destinationState]]);
                minimizedTransitions.push_back(fromState + " " + toState + " " + string(1, letter));
            }
        }

        string minimizedInitialState = initialState;
        if (stateToPartition.count(initialState)) {
            minimizedInitialState = getPartitionName(partitions[stateToPartition[initialState]]);
        }

        return AutomatDFA(minimizedStates.size(), minimizedStates,
                          minimizedTransitions.size(), minimizedTransitions,
                          minimizedInitialState, minimizedFinalStates.size(),
                          minimizedFinalStates);
    }
};

#endif
