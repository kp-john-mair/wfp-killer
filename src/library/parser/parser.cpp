#include <parser/parser.h>

namespace wfpk {
class Parser
{
public:
    explicit Parser(Lexer &lexer)
    : _lexer{lexer}
    {}

public:
    // Parse the token stream
    void parse();
    // Parse the token stream with verbose tracing
    void parseTrace()
    {
        _shouldTrace = true;
        return parse();
    }

private:
    // Token processing
    // Asserts that a given token must come next and increments the cursor on success
    // throws a ParseError on failure
    void match(TokenType type);
    // Consumes one token and moves the cursor
    void consume()
    {
        _lookahead = _lexer.nextToken();
    }

private:
    Lexer &_lexer;
    Token _lookahead{};
    bool _shouldTrace{false};
};
}
