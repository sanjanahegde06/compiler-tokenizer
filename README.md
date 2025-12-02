# Compiler Tokenizer Project

A compact, beginner-friendly C++ tokenizer that reads source from `input.code` and prints a table of tokens and their types.

Features
- Identifies Keywords, Identifiers, Numbers (integers and decimals), Operators, Delimiters, and Strings.
- Ignores whitespace and comments (`//` single-line and `/* ... */` multi-line).
- Clean, modular code with separate functions: `isKeyword()`, `isOperator()`, `isDelimiter()`, and `tokenize()`.

Build

Use g++ to compile:

```
g++ main.cpp -o tokenizer
```

Run

On Windows PowerShell:

```
.\tokenizer
```

On Unix-like shells:

```
./tokenizer
```

Files
- `main.cpp` : Tokenizer implementation (contains comments and explanations).
- `input.code` : Example input program to tokenize.
- `example_output.txt` : Example output produced by the tokenizer for `input.code`.

How it works (brief)
- The program reads `input.code` entirely into a string.
- `tokenize()` walks the source character-by-character and:
  - Skips whitespace.
  - Skips comments (`//` and `/* ... */`).
  - Extracts strings delimited by `"` and handles escaped characters.
  - Identifies identifiers/keywords (letters and underscores followed by letters/digits/underscores).
  - Parses numbers (supports a single decimal point).
  - Detects multi-character operators (`==`, `!=`, `<=`, `>=`) before single-character operators.
  - Recognizes delimiters `; , ( ) { } [ ]`.
- The program produces a formatted table of tokens and their type.
