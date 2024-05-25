#include <iostream>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <cctype>

// Enum class for token types
enum class TokenType {
    DictionaryOpen,  // "<dictionary> {"
    ArrayOpen,       // "<array> {"
    BlockClose,      // "}"
    Colon,           // ":"
    Identifier,      // Key or Item names
    NewLine,
    NoKeyError,      // Represents a failure by scutil to lookup a key, see Lexer::NoKeyErrorText
    EndOfInput
};

// Represents a token
struct Token {
    TokenType type{};
    std::string text;
};

// Associate the name of token with the token type
std::unordered_map<TokenType, std::string> tokenMap {
    {TokenType::DictionaryOpen, "DictionaryOpen"},
    {TokenType::ArrayOpen, "ArrayOpen"},
    {TokenType::BlockClose, "BlockClose"},
    {TokenType::Colon, "Colon"},
    {TokenType::Identifier, "Identifier"},
    {TokenType::NewLine, "NewLine"},
    {TokenType::NoKeyError, "NoKeyError"},
    {TokenType::EndOfInput, "EndOfInput"}
};

// Custom error that represents a parsing/lexing failure
class ParseError : public std::runtime_error {
public:
    explicit ParseError(const std::string &reason)
        : std::runtime_error(reason) {}
};

// The Lexer is responsible for breaking up a string of text into tokens
class Lexer 
{
    // Lexemes used to match tokens
    inline static const std::string DictionaryOpenText = "<dictionary> {";
    inline static const std::string ArrayOpenText = "<array> {";
    inline static const std::string BlockCloseText = "}";
    inline static const std::string ColonText = " : ";
    inline static const std::string NewLineText = "\n";
    inline static const std::string NoKeyErrorText = "  No such key\n";

public:
    explicit Lexer(const std::string &input)
        : _input{input}, _currentIndex{0} {
        // Lexable content must either start with a container "<" or
        // have the key error content
        if(!_input.starts_with("<") && !_input.starts_with(NoKeyErrorText)) {
            throw ParseError("Unlexable content, got input: " + _input);
        }
    }

    // Return the next available token from the input
    Token nextToken();

private:
    std::string _input;
    size_t _currentIndex;
};

Token Lexer::nextToken() {
    // End of input
    if (_currentIndex >= _input.length())
        return {TokenType::EndOfInput, "EOF"};

    // Tokenize a dictionary start
    if (_input.compare(_currentIndex, DictionaryOpenText.length(), DictionaryOpenText) == 0) {
        _currentIndex += DictionaryOpenText.length();
        return {TokenType::DictionaryOpen, DictionaryOpenText};
    }
    // Tokenize an array start
    else if (_input.compare(_currentIndex, ArrayOpenText.length(), ArrayOpenText) == 0) {
        _currentIndex += ArrayOpenText.length();
        return {TokenType::ArrayOpen, ArrayOpenText};
    }
    // Tokenize a block close ("}")
    else if (_input[_currentIndex] == BlockCloseText[0]) {
        _currentIndex++;
        return {TokenType::BlockClose, BlockCloseText};
    }
    // Tokenize a colon - represents the separator for key/values in a dictionary or array
    else if (_input.compare(_currentIndex, ColonText.length(), ColonText) == 0) {
        _currentIndex += ColonText.length();
        return {TokenType::Colon, ColonText};
    }
    // Newlines are tokens as they're significant in this grammar
    else if (_input[_currentIndex] == '\n') {
        // After a new-line, clear out the white-space until the next lexeme.
        while (_currentIndex < _input.length() && std::isspace(_input[_currentIndex])) {
            _currentIndex++;
        }
        return {TokenType::NewLine, "NewLine"};
    }
    // Tokenize a No Key Error - occurs when scutil can't find a key for a dict
    else if (_input.compare(_currentIndex, NoKeyErrorText.length(), NoKeyErrorText) == 0) {
        _currentIndex += NoKeyErrorText.length();
        return {TokenType::NoKeyError, "NoKeyError"};
    }
    // Treat all remaining text as "identifiers", these include dictionary keys, values, and array indices
    else {
        size_t start = _currentIndex;

        // Identifiers cannot contain spaces, read remaining text until the next space
        while (_currentIndex < _input.length() && !std::isspace(_input[_currentIndex])) {
            _currentIndex++;
        }

        return {TokenType::Identifier, _input.substr(start, _currentIndex - start)};
    }
}