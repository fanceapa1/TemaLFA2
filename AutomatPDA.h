#ifndef AUTOMAT_PDA_H
#define AUTOMAT_PDA_H

#include <cctype>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

class AutomatPDA {
    const string FINAL_STATE = "final_state";
    const string EMPTY_STACK = "empty_stack";
    const string BOTH = "both";

    struct Transition {
        string currentState;
        string inputSymbol;
        string stackTop;
        string nextState;
        string pushString;
    };

    struct Configuration {
        string state;
        int wordIndex;
        vector<string> stack;
    };

    set<string> states;
    set<string> inputAlphabet;
    set<string> stackAlphabet;
    vector<Transition> transitions;
    string initialState;
    string initialStackSymbol;
    set<string> finalStates;
    string acceptingMethod;

    string toLowerAscii(string text) const {
        for (char& character : text) {
            character = static_cast<char>(tolower(static_cast<unsigned char>(character)));
        }

        return text;
    }

    string parseAcceptingMethod(const string& acceptingMethodText) const {
        string normalizedMethod = toLowerAscii(acceptingMethodText);

        if (normalizedMethod.find("ambele") != string::npos) {
            return BOTH;
        }
        if (normalizedMethod.find("final") != string::npos) {
            return FINAL_STATE;
        }
        if (normalizedMethod.find("stiv") != string::npos ||
            normalizedMethod.find("goal") != string::npos) {
            return EMPTY_STACK;
        }

        return FINAL_STATE;
    }

    bool isAcceptingConfiguration(const Configuration& configuration, int wordLength) const {
        if (configuration.wordIndex != wordLength) {
            return false;
        }

        bool isInFinalState = finalStates.count(configuration.state) > 0;
        bool hasEmptyStack = configuration.stack.empty();

        if (acceptingMethod == FINAL_STATE) {
            return isInFinalState;
        }
        if (acceptingMethod == EMPTY_STACK) {
            return hasEmptyStack;
        }

        return isInFinalState && hasEmptyStack;
    }

    string getConfigurationKey(const Configuration& configuration) const {
        string key = configuration.state + "|" + to_string(configuration.wordIndex) + "|";

        for (const string& stackSymbol : configuration.stack) {
            key += stackSymbol + ",";
        }

        return key;
    }

    void pushSymbols(vector<string>& stack, const string& pushString) const {
        if (pushString == "lambda") {
            return;
        }

        for (int i = static_cast<int>(pushString.size()) - 1; i >= 0; i--) {
            stack.push_back(string(1, pushString[i]));
        }
    }

public:
    AutomatPDA(const set<string>& states, const set<string>& inputAlphabet,
               const set<string>& stackAlphabet, const vector<string>& transitionLines,
               const string& initialState, const string& initialStackSymbol,
               const set<string>& finalStates,
               const string& acceptingMethodText)
        : states(states), inputAlphabet(inputAlphabet),
          stackAlphabet(stackAlphabet), initialState(initialState),
          initialStackSymbol(initialStackSymbol), finalStates(finalStates),
          acceptingMethod(parseAcceptingMethod(acceptingMethodText))
    {
        for (const string& transitionLine : transitionLines) {
            stringstream ss(transitionLine);
            Transition transition;

            ss >> transition.currentState >> transition.inputSymbol
               >> transition.stackTop >> transition.nextState
               >> transition.pushString;

            transitions.push_back(transition);
        }
    }

    bool wordIsAccepted(const string& word) const {
        vector<Configuration> configurationsToProcess;
        set<string> visitedConfigurations;

        configurationsToProcess.push_back({initialState, 0, {initialStackSymbol}});

        while (!configurationsToProcess.empty()) {
            Configuration currentConfiguration = configurationsToProcess.back();
            configurationsToProcess.pop_back();

            string configurationKey = getConfigurationKey(currentConfiguration);
            if (!visitedConfigurations.insert(configurationKey).second) {
                continue;
            }

            if (isAcceptingConfiguration(currentConfiguration, word.length())) {
                return true;
            }

            for (const Transition& transition : transitions) {
                if (transition.currentState != currentConfiguration.state) {
                    continue;
                }
                if (currentConfiguration.stack.empty() ||
                    currentConfiguration.stack.back() != transition.stackTop) {
                    continue;
                }

                bool consumesInput = transition.inputSymbol != "lambda";
                int nextWordIndex = currentConfiguration.wordIndex;

                if (consumesInput) {
                    if (nextWordIndex >= word.length() ||
                        transition.inputSymbol != string(1, word[nextWordIndex])) {
                        continue;
                    }

                    nextWordIndex++;
                }

                Configuration nextConfiguration;
                nextConfiguration.state = transition.nextState;
                nextConfiguration.wordIndex = nextWordIndex;
                nextConfiguration.stack = currentConfiguration.stack;
                nextConfiguration.stack.pop_back();
                pushSymbols(nextConfiguration.stack, transition.pushString);

                configurationsToProcess.push_back(nextConfiguration);
            }
        }

        return false;
    }
};

#endif
