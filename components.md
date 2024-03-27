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
