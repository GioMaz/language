# Lexer

The main focus of the lexer is to transform a string
of source code into a vector of tokens. The pseudo-
code of the lexer is the following:

```
class Lexer {
    int pos
    string source
    Token tokens
}

bool get_token(Lexer l)
{
    char c = l.source.getchar()
    match c {
    case '+':
        l.tokens.add(PLUS)
        break;
    case '-':
        l.tokens.add(MINUS)
        break;
    ...
    case EOF:
        return false
    default:
        if (is_digit(l)):
            int n = get_number(l)
            l.tokens.add(n)
        else if (is_alpha(l)):
            string s = get_string(l)
            l.tokens.add(s)
        else:
            return false
    }
    return true
}

void get_tokens(Lexer l)
{
    while (get_token(l)) {
        l.pos++
    }
}

```
