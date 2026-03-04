#include <iostream>
#include <string>
#include <cctype>
#include <stdexcept> 

using namespace std;

class CompilerParser {
    string expr;
    int pos;

    char peek() {
        if (pos < expr.length()) return expr[pos];
        return '\0';
    }

    char get() {
        if (pos < expr.length()) return expr[pos++];
        return '\0';
    }

    double parseNumber() {
        if (!isdigit(peek())) {
            throw runtime_error("Expected a number but found invalid syntax.");
        }
        double result = 0;
        while (isdigit(peek())) {
            result = result * 10 + (get() - '0');
        }
        return result;
    }

    double parseFactor() {
        if (peek() == '(') {
            get(); 
            double result = parseExpression();
            if (peek() != ')') {
                throw runtime_error("Mismatched parentheses: Missing ')'");
            }
            get(); 
            return result;
        }
        return parseNumber();
    }

    double parseTerm() {
        double result = parseFactor();
        while (peek() == '*' || peek() == '/') {
            char op = get();
            double nextFactor = parseFactor();
            if (op == '/') {
                if (nextFactor == 0) {
                    throw runtime_error("Math Error: Division by zero is undefined.");
                }
                result /= nextFactor;
            } else {
                result *= nextFactor;
            }
        }
        return result;
    }

public:
    CompilerParser(string e) : pos(0) {
        for (char c : e) {
            if (!isspace(c)) {
                if (!isdigit(c) && c != '+' && c != '-' && c != '*' && c != '/' && c != '(' && c != ')') {
                    throw runtime_error(string("Lexical Error: Unrecognized character '") + c + "'");
                }
                expr += c;
            }
        }
    }

    double parseExpression() {
        double result = parseTerm();
        while (peek() == '+' || peek() == '-') {
            char op = get();
            double nextTerm = parseTerm();
            if (op == '+') result += nextTerm;
            else if (op == '-') result -= nextTerm;
        }
        return result;
    }
    
    double evaluate() {
        if (expr.empty()) throw runtime_error("No expression provided.");
        double result = parseExpression();
        if (pos < expr.length()) {
            throw runtime_error("Syntax Error: Unexpected characters at the end of the expression.");
        }
        return result;
    }
};

int main() {
    string input;
    cout << "Enter an arithmetic expression: ";
    if (getline(cin, input)) {
        try {
            CompilerParser parser(input);
            cout << "Parsed Result: " << parser.evaluate() << endl;
        } catch (const exception& e) {
            cout << "COMPILATION FAILED -> " << e.what() << endl;
        }
    }
    return 0;
}