#include <parser/parser.h>

namespace wfpk {
auto Parser::match(TokenType type) -> std::optional<Token>
{
    std::string tokenStr{enumName(type)};
    if(_shouldTrace)
        std::cout << "Looking for a token of type:" << tokenStr;


    // Save current token
    Token currentToken = _lookahead;

    if(_lookahead.type == type)
    {
        if(_shouldTrace)
            std::cout << "Matched a token of type:" << tokenStr << "text: " << _lookahead.text;

        // Move the input to the next token
        consume();

        return currentToken;;
    }
    else
    {
        return {};
    }
}

auto Parser::numberList() -> std::vector<uint16_t>
{
    std::vector<uint16_t> numbers;

    mustMatch(TokenType::LBrack);
    while(true)
    {
        if(auto tok = match(TokenType::Number))
        {
            numbers.push_back(static_cast<uint16_t>(std::atoi(tok->text)));
            if(match(TokenType::Comma))
                consume(); // advance by one token
            else if(match(TokenType::RBrack))
                return numbers;
            else
                unexpectedTokenError();
        }
        // This will only trigger in the case of an empty list `{}`
        else if(match(TokenType::RBrack))
        {
            return numbers;
        }
        else
        {
            unexpectedTokenError();
        }
    }

    return numbers;
}

auto Parser::addressAndPorts() -> std::pair<std::string, std::vector<uint16_t>>
{
    std::string address;
    std::vector<uint16_t> ports{0};

    if(auto tok = match(TokenType::Ipv4Address, TokenType::Ipv6Address))
        address = tok->text;

    if(match(TokenType::Port))
    {
        if(auto tok = match(TokenType::Number))
        {
            ports = static_cast<uint16_t>(std::atoi(tok->text));
        }
        else if(peek(TokenType::LBrack))
            auto ports = numberList();

    }

}

void Parser::sourceCondition(FilterConditions *pConditions)
{

    if(auto tok = match(TokenType::String))
    {
        pConditions->sourceApp = tok->text;

        // If we have a source app, we can't have any further conditions!
        // we don't currently allow a source app to be constrained by port or ip
        return;
    }
    else
    {
    if(auto tok = match(TokenType::Ipv4Address, TokenType::Ipv6Address))
    {
        pConditions->sourceIp = tok->text;
    }

    if(match(TokenType::Port))
    {
        if(auto tok = match(TokenType::Number))
            pConditions->sourcePorts.push_back(static_cast<uint32_t>(std::atoi(tok->text.c_str())));
    }
}

FilterConditions Parser::conditions()
{
    if(match(TokenType::All))
        return NoFilterConditions;

    FilterConditions conditions{};

    if(auto tok = match(TokenType::Inet4, TokenType::Inet6))
    {
        if(tok.type == TokenType::Inet4)
            conditions.ipVersion = FilterConditions::Inet4;
        else
            conditions.ipVersion = FilterConditions::Inet6;
    }
    else if(match(TokenType::From))
    {
        sourceCondition(&conditions);
    }
    else if(match(TokenType::To))
        destinationCondition(&conditions);
     // foo

}

std::unique_ptr<Node> Parser::filter()
{
    FilterNode::Action action{};
    FilterNode::Layer layer{};

    if(match(TokenType::PermitAction))
        action = FilterNode::Permit;
    else if(match(TokenType::BlockAction))
        action = FilterNode::Block;
    else
        throw ParseError(std::format("Expected block or permit, but got: {}", peek().description()));

    if(match(TokenType::OutDir))
        // cannot actually decice the layer until the "conditions" have been parsed
        // and we know wther it's ipv4 or ipv6 or both (i.e inet vs inet6)
        // instead what this does is constrains the layers that are available to the conditions
        // so if it's 'out' then we only get AUTH_CONNECT_V* NOT the AUTH_RECV_V*
        layer = FilterNode::AUTH_CONNECT_V4;
    else if(match(TokenType::InDir))
        layer = FilterNode::AUTH_CONNECT_V4;
    else
        throw ParseError(std::format("Expected in or out, but got: {}", peek().description()));


    conditions();

    conditions(dict);
    match(TokenType::BlockClose);

    if(dict.contains(QLatin1String("PIAEmpty")))
        return QJsonValue::Null;
    else
        return dict;
}

std::unique_ptr<Node> Parser::parse()
{
    using enum TokenType;
    try
    {
        // Get initial token
        consume();

        if(_lookahead.type == TokenType::PermitAction)
        {
            auto dict = filter();
            return dict;
        }
    }
    catch(const std::exception &ex)
    {
        qCritical() << "Failed to parse scutil output: " << ex.what();
        return QJsonValue::Undefined;
    }
    catch(...)
    {
        qCritical() << "Failed to parse scutil output: Unknown error";
        return QJsonValue::Undefined;
    }
}
}
