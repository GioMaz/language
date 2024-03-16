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
 - Don't allocate tokens in the heap
 - Don't use the same struct between parser and interpreter (use token and value structs)
 - Use pointers to the stmt struct instead of pointers to the anystmt struct
