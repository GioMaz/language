```
        +-----------+
        |   Lexer   |
        +-----------+
              |
        +-----------+
        |   Parser  |
        +-----------+
              |
      +-------+-------+
      |               |
+-----------+   +-----------+
|Interpreter|   |  Semantic |
+-----------+   |  Analyzer |
                +-----------+
                      |
                +-----------+
                |    Code   |
                | Generator |
                +-----------+
```

### BAD CHOICES:
 - Don't allocate token data in the heap
 - Don't use the same struct between parser and interpreter (use Token and Value structs)
 - Use pointers to the stmt struct instead of pointers to the anystmt struct

### 2nd bad choice explained
Suppose that I'm doing a compiler, I have to implement a lexer and a parser, is it the lexer's or parser's job to convert strings from the source code to string and number literals?

In a typical compiler architecture, the lexer and parser have distinct responsibilities:

    Lexer (Tokenizer): The lexer's primary responsibility is to break down the input source code into a sequence of tokens. These tokens are the smallest units of meaning in the programming language, such as keywords, identifiers, operators, and literals. The lexer typically recognizes different types of literals (e.g., string literals, numeric literals) but doesn't convert them into their final form.

    Parser: The parser's job is to analyze the sequence of tokens produced by the lexer and build a parse tree or abstract syntax tree (AST) representing the syntactic structure of the program. The parser ensures that the sequence of tokens conforms to the grammar rules of the programming language. It doesn't deal with the actual values of literals; rather, it's concerned with the structure and relationships between tokens.

So, to directly answer your question, it's usually the lexer's responsibility to recognize and tokenize string and number literals from the source code. However, the lexer typically doesn't perform the conversion of these literals into their corresponding data types (e.g., converting a string literal "123" into the integer 123). This conversion typically occurs later in the compilation process, possibly during semantic analysis or code generation phases.
