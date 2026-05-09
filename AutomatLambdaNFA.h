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

    static bool hasTopLevelUnion(const string& expression) {
        int parenthesesDepth = 0;

        for (char character : expression) {
            if (character == '(') {
                parenthesesDepth++;
            }
            else if (character == ')') {
                parenthesesDepth--;
            }
            else if (character == '|' && parenthesesDepth == 0) {
                return true;
            }
        }

        return false;
    }

    static vector<string> splitTopLevelUnion(const string& expression) {
        vector<string> alternatives;
        int parenthesesDepth = 0;
        string currentAlternative;

        for (char character : expression) {
            if (character == '(') {
                parenthesesDepth++;
            }
            else if (character == ')') {
                parenthesesDepth--;
            }

            if (character == '|' && parenthesesDepth == 0) {
                alternatives.push_back(currentAlternative);
                currentAlternative.clear();
            }
            else {
                currentAlternative += character;
            }
        }

        alternatives.push_back(currentAlternative);
        return alternatives;
    }

    static bool hasWrappingParentheses(const string& expression) {
        if (expression.size() < 2 || expression.front() != '(' || expression.back() != ')') {
            return false;
        }

        int parenthesesDepth = 0;

        for (int i = 0; i < expression.size(); i++) {
            if (expression[i] == '(') {
                parenthesesDepth++;
            }
            else if (expression[i] == ')') {
                parenthesesDepth--;
            }

            if (parenthesesDepth == 0 && i != expression.size() - 1) {
                return false;
            }
        }

        return true;
    }

    static string wrapForConcatenation(const string& expression) {
        if (hasTopLevelUnion(expression)) {
            return "(" + expression + ")";
        }

        return expression;
    }

    static string wrapForStar(const string& expression) {
        if (expression.size() == 1 || expression == "lambda" || expression == "empty" ||
            hasWrappingParentheses(expression)) {
            return expression;
        }

        return "(" + expression + ")";
    }

    static string unionRegex(const string& firstExpression, const string& secondExpression) {
        if (firstExpression == "empty") {
            return secondExpression;
        }
        if (secondExpression == "empty") {
            return firstExpression;
        }
        if (firstExpression == secondExpression) {
            return firstExpression;
        }

        vector<string> alternatives;
        set<string> seenAlternatives;

        for (const string& expression : {firstExpression, secondExpression}) {
            for (const string& alternative : splitTopLevelUnion(expression)) {
                if (alternative != "empty" && seenAlternatives.insert(alternative).second) {
                    alternatives.push_back(alternative);
                }
            }
        }

        if (alternatives.empty()) {
            return "empty";
        }

        string result = alternatives[0];
        for (int i = 1; i < alternatives.size(); i++) {
            result += "|" + alternatives[i];
        }

        return result;
    }

    static string concatenateRegex(const vector<string>& expressions) {
        string result;

        for (const string& expression : expressions) {
            if (expression == "empty") {
                return "empty";
            }
            if (expression == "lambda") {
                continue;
            }

            result += wrapForConcatenation(expression);
        }

        if (result.empty()) {
            return "lambda";
        }

        return result;
    }

    static string starRegex(const string& expression) {
        if (expression == "empty" || expression == "lambda") {
            return "lambda";
        }

        return wrapForStar(expression) + "*";
    }

    static string getRegexTransition(const map<string, map<string, string>>& regexTransitions,
                              const string& fromState, const string& toState) {
        if (!regexTransitions.count(fromState) || !regexTransitions.at(fromState).count(toState)) {
            return "empty";
        }

        return regexTransitions.at(fromState).at(toState);
    }

    static string getUniqueStateName(const string& baseName, const set<string>& currentStates) {
        string stateName = baseName;
        int suffix = 1;

        while (currentStates.count(stateName)) {
            stateName = baseName + to_string(suffix);
            suffix++;
        }

        return stateName;
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

    string toRegularExpression() const {
        AutomatDFA dfa = this->transformToDFA();

        if (dfa.finalStates.empty()) {
            return "empty";
        }

        set<string> regexStates = dfa.states;
        set<string> regexFinalStates = dfa.finalStates;
        string regexInitialState = dfa.initialState;
        map<string, map<string, string>> regexTransitions;

        for (const auto& stateTransitions : dfa.transitionTable) {
            const string& fromState = stateTransitions.first;

            for (const auto& letterTransition : stateTransitions.second) {
                string transitionRegex(1, letterTransition.first);
                const string& toState = letterTransition.second;
                regexTransitions[fromState][toState] =
                    unionRegex(getRegexTransition(regexTransitions, fromState, toState), transitionRegex);
            }
        }

        bool initialStateHasIncomingTransitions = false;
        for (const auto& stateTransitions : regexTransitions) {
            for (const auto& transition : stateTransitions.second) {
                if (transition.first == regexInitialState) {
                    initialStateHasIncomingTransitions = true;
                    break;
                }
            }

            if (initialStateHasIncomingTransitions) {
                break;
            }
        }

        if (regexFinalStates.count(regexInitialState) || initialStateHasIncomingTransitions) {
            string newInitialState = getUniqueStateName("__regex_start", regexStates);
            regexStates.insert(newInitialState);
            regexTransitions[newInitialState][regexInitialState] =
                unionRegex(getRegexTransition(regexTransitions, newInitialState, regexInitialState), "lambda");
            regexInitialState = newInitialState;
        }

        bool singleFinalStateHasOutgoingTransitions = false;
        if (regexFinalStates.size() == 1) {
            const string& finalState = *regexFinalStates.begin();
            singleFinalStateHasOutgoingTransitions =
                regexTransitions.count(finalState) && !regexTransitions[finalState].empty();
        }

        if (regexFinalStates.size() > 1 || singleFinalStateHasOutgoingTransitions) {
            string newFinalState = getUniqueStateName("__regex_final", regexStates);
            regexStates.insert(newFinalState);

            for (const string& finalState : regexFinalStates) {
                regexTransitions[finalState][newFinalState] =
                    unionRegex(getRegexTransition(regexTransitions, finalState, newFinalState), "lambda");
            }

            regexFinalStates.clear();
            regexFinalStates.insert(newFinalState);
        }

        const string& regexFinalState = *regexFinalStates.begin();
        vector<string> statesToEliminate;

        for (const string& state : regexStates) {
            if (state != regexInitialState && state != regexFinalState) {
                statesToEliminate.push_back(state);
            }
        }

        for (const string& eliminatedState : statesToEliminate) {
            string loopRegex = getRegexTransition(regexTransitions, eliminatedState, eliminatedState);
            string loopStarRegex = starRegex(loopRegex);

            for (const string& fromState : regexStates) {
                if (fromState == eliminatedState) {
                    continue;
                }

                string intoEliminatedRegex = getRegexTransition(regexTransitions, fromState, eliminatedState);
                if (intoEliminatedRegex == "empty") {
                    continue;
                }

                for (const string& toState : regexStates) {
                    if (toState == eliminatedState) {
                        continue;
                    }

                    string outOfEliminatedRegex = getRegexTransition(regexTransitions, eliminatedState, toState);
                    if (outOfEliminatedRegex == "empty") {
                        continue;
                    }

                    string newPathRegex = concatenateRegex({intoEliminatedRegex, loopStarRegex, outOfEliminatedRegex});
                    regexTransitions[fromState][toState] =
                        unionRegex(getRegexTransition(regexTransitions, fromState, toState), newPathRegex);
                }
            }

            regexTransitions.erase(eliminatedState);
            for (auto& stateTransitions : regexTransitions) {
                stateTransitions.second.erase(eliminatedState);
            }
            regexStates.erase(eliminatedState);
        }

        return getRegexTransition(regexTransitions, regexInitialState, regexFinalState);
    }
};

#endif
