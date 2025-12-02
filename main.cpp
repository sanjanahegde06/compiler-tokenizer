// Compiler Tokenizer Project (Improved)
// main.cpp
// A readable tokenizer (lexical analyzer) for a C-like subset.
// This version improves handling of: char literals, string escapes,
// numbers (including .45 and scientific notation), operators, keywords,
// identifiers, and comments. It also reports line numbers and uses a
// TokenType enum for clearer code.

#include <bits/stdc++.h>
using namespace std;

// ---------- Token types ----------
enum class TokenType {
    Keyword,
    Identifier,
    Number,
    Operator,
    Delimiter,
    String,
    Char,
    Unknown
};

static string tokenTypeToString(TokenType t) {
    switch (t) {
        case TokenType::Keyword: return "Keyword";
        case TokenType::Identifier: return "Identifier";
        case TokenType::Number: return "Number";
        case TokenType::Operator: return "Operator";
        case TokenType::Delimiter: return "Delimiter";
        case TokenType::String: return "String";
        case TokenType::Char: return "Char";
        default: return "Unknown";
    }
}

// A simple Token struct with lexeme, type and line number
struct Token {
    string lexeme;
    TokenType type;
    int line;
};

// ---------- Classification helpers ----------

bool isKeyword(const string &s) {
    static const unordered_set<string> keywords = {
        // types
        "int", "float", "double", "char", "long", "short", "bool", "void",
        // control
        "if", "else", "for", "while", "do", "return", "switch", "case", "break", "continue",
        // declarations / OOP
        "class", "struct", "public", "private", "protected",
        // preprocessor / namespaces
        "include", "namespace", "using",
    };
    return keywords.find(s) != keywords.end();
}

bool isDelimiter(char c) {
    static const string delims = ";,(){}[]";
    return delims.find(c) != string::npos;
}

// Operators set (single, double and some triple-length like <<=)
bool isOperatorString(const string &s) {
    static const unordered_set<string> ops = {
        // triple and double char operators
        "<<=", ">>=",
        "==", "!=", "<=", ">=", "++", "--", "+=", "-=", "*=", "/=", "%=", "<<", ">>",
        "&&", "||",
        // single char operators
        "+", "-", "*", "/", "%", "=", "<", ">", "!", "&", "|", "^", "~"
    };
    return ops.find(s) != ops.end();
}

// ---------- Tokenizer implementation ----------

// Helper: peek ahead safely
inline char peekChar(const string &s, size_t i, int offset = 0) {
    size_t idx = i + offset;
    return idx < s.size() ? s[idx] : '\0';
}

// Parse a character literal starting at i (where s[i] == '\'')
// Returns the lexeme and advances index (by reference) and updates line count for embedded newlines.
static string parseCharLiteral(const string &s, size_t &i, int &line) {
    // Assumes s[i] == '\''
    string lexeme;
    size_t n = s.size();
    lexeme.push_back(s[i]); // opening '
    ++i;

    if (i >= n) return lexeme; // malformed, return what we have

    if (s[i] == '\\') {
        // escaped sequence: include backslash and next char if any
        lexeme.push_back(s[i]);
        ++i;
        if (i < n) {
            lexeme.push_back(s[i]);
            ++i;
        }
    } else {
        // normal character (could be anything except newline)
        if (s[i] == '\n') {
            // newline inside char literal - malformed, but include and bump line
            ++line;
            lexeme.push_back(s[i]);
            ++i;
        } else {
            lexeme.push_back(s[i]);
            ++i;
        }
    }

    // Consume closing quote if present
    if (i < n && s[i] == '\'') {
        lexeme.push_back(s[i]);
        ++i;
    }

    return lexeme;
}

// Parse string literal starting at i (s[i] == '"')
static string parseStringLiteral(const string &s, size_t &i, int &line) {
    string lexeme;
    size_t n = s.size();
    lexeme.push_back(s[i]); // opening quote
    ++i;

    while (i < n) {
        char c = s[i];
        lexeme.push_back(c);
        ++i;
        if (c == '\\') {
            // escaped char - include next char without interpretation
            if (i < n) {
                lexeme.push_back(s[i]);
                ++i;
            }
            continue;
        }
        if (c == '"') break; // end of string
        if (c == '\n') ++line; // count lines inside string
    }

    return lexeme;
}

// Parse a number (supports: 123, 12.34, .45, 1e10, 1.2e-3)
static string parseNumber(const string &s, size_t &i) {
    string lexeme;
    size_t n = s.size();
    bool hasDigits = false;

    // Integer part (optional if starts with .)
    while (i < n && isdigit(static_cast<unsigned char>(s[i]))) {
        hasDigits = true;
        lexeme.push_back(s[i]);
        ++i;
    }

    // Fractional part
    if (i < n && s[i] == '.') {
        lexeme.push_back('.');
        ++i;
        // digits after dot
        while (i < n && isdigit(static_cast<unsigned char>(s[i]))) {
            hasDigits = true;
            lexeme.push_back(s[i]);
            ++i;
        }
    }

    // Exponent part
    if (i < n && (s[i] == 'e' || s[i] == 'E')) {
        size_t save = i;
        lexeme.push_back(s[i]);
        ++i;
        if (i < n && (s[i] == '+' || s[i] == '-')) {
            lexeme.push_back(s[i]);
            ++i;
        }
        bool expDigits = false;
        while (i < n && isdigit(static_cast<unsigned char>(s[i]))) {
            expDigits = true;
            lexeme.push_back(s[i]);
            ++i;
        }
        if (!expDigits) {
            // rollback exponent if no digits followed
            i = save;
            lexeme.erase(lexeme.size() - 1); // remove the 'e' or 'E'
        }
    }

    // Note: If lexeme is just "." (no digits) then it's not a number.
    return lexeme;
}

// Main tokenize function: returns vector of Token (lexeme/type/line)
vector<Token> tokenize(const string &code) {
    vector<Token> tokens;
    size_t n = code.size();
    size_t i = 0;
    int line = 1;

    while (i < n) {
        char c = code[i];

        // Whitespace handling: track line numbers
        if (isspace(static_cast<unsigned char>(c))) {
            if (c == '\n') ++line;
            ++i;
            continue;
        }

        // Comments: single-line // or multi-line /* */ - skip entirely
        if (c == '/' && peekChar(code, i, 1) == '/') {
            // single-line comment
            i += 2;
            while (i < n && code[i] != '\n') ++i;
            continue; // next loop will consume newline and increment line
        }

        if (c == '/' && peekChar(code, i, 1) == '*') {
            // multi-line comment
            i += 2;
            while (i + 1 < n && !(code[i] == '*' && code[i+1] == '/')) {
                if (code[i] == '\n') ++line;
                ++i;
            }
            if (i + 1 < n) i += 2; // skip closing */
            continue;
        }

        // Char literal
        if (c == '\'') {
            string lex = parseCharLiteral(code, i, line);
            tokens.push_back({lex, TokenType::Char, line});
            continue;
        }

        // String literal
        if (c == '"') {
            string lex = parseStringLiteral(code, i, line);
            tokens.push_back({lex, TokenType::String, line});
            continue;
        }

        // Identifier or keyword: start with letter or underscore
        if (isalpha(static_cast<unsigned char>(c)) || c == '_') {
            string id;
            while (i < n && (isalnum(static_cast<unsigned char>(code[i])) || code[i] == '_')) {
                id.push_back(code[i]);
                ++i;
            }
            if (isKeyword(id)) tokens.push_back({id, TokenType::Keyword, line});
            else tokens.push_back({id, TokenType::Identifier, line});
            continue;
        }

        // Number: starts with digit, or a dot followed by digit
        if (isdigit(static_cast<unsigned char>(c)) || (c == '.' && isdigit(static_cast<unsigned char>(peekChar(code, i, 1))))) {
            string num = parseNumber(code, i);
            if (!num.empty() && (any_of(num.begin(), num.end(), ::isdigit))) {
                tokens.push_back({num, TokenType::Number, line});
                continue;
            }
            // If parseNumber failed (just a dot), fall through to operator/delimiter handling
        }

        // Operators: try longest match (3, then 2, then 1)
        bool matchedOp = false;
        for (int len = 3; len >= 1; --len) {
            string s;
            for (int k = 0; k < len; ++k) {
                char ch = peekChar(code, i, k);
                if (ch == '\0') { s.clear(); break; }
                s.push_back(ch);
            }
            if (!s.empty() && isOperatorString(s)) {
                tokens.push_back({s, TokenType::Operator, line});
                i += s.size();
                matchedOp = true;
                break;
            }
        }
        if (matchedOp) continue;

        // Delimiters
        if (isDelimiter(c)) {
            tokens.push_back({string(1, c), TokenType::Delimiter, line});
            ++i;
            continue;
        }

        // Unknown single character (capture and move on)
        tokens.push_back({string(1, c), TokenType::Unknown, line});
        ++i;
    }

    return tokens;
}

// ---------- Main: read file, tokenize, and print results ----------

int main(int argc, char **argv) {
    // Behavior:
    // - If a filename is provided as first argument, read that file.
    // - If no arguments are provided, read source from stdin (so you can pipe or paste code directly).

    string source;
    if (argc > 1) {
        string filename = argv[1];
        if (filename == "-") {
            // Read from stdin
            stringstream buffer;
            buffer << cin.rdbuf();
            source = buffer.str();
        } else {
            ifstream in(filename);
            if (!in) {
                cerr << "Error: could not open '" << filename << "' for reading.\n";
                return 1;
            }
            stringstream buffer;
            buffer << in.rdbuf();
            source = buffer.str();
        }
    } else {
        // No filename -> read from stdin (useful for piping or here-strings)
        stringstream buffer;
        buffer << cin.rdbuf();
        source = buffer.str();
    }

    // Tokenize
    auto tokens = tokenize(source);

    // Print the required check lines
    cout << "\u2714 Tokens found\n";       // ✔
    cout << "\u2714 Type of token\n\n"; // ✔

    // Print table header: Token | Type | Line
    const int tokWidth = 30;
    const int typeWidth = 15;
    const int lineWidth = 6;
    cout << left << setw(tokWidth) << "Token" << " | " << left << setw(typeWidth) << "Type" << " | " << left << setw(lineWidth) << "Line" << "\n";
    cout << string(tokWidth, '-') << "-|" << string(typeWidth, '-') << "-|" << string(lineWidth, '-') << "\n";

    for (auto &t : tokens) {
        cout << left << setw(tokWidth) << t.lexeme << " | " << left << setw(typeWidth) << tokenTypeToString(t.type) << " | " << left << setw(lineWidth) << t.line << "\n";
    }

    return 0;
}
