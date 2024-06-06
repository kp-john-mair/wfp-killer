#include <parser/parser.h>

namespace wfpk {
auto Parser::match(TokenType type) -> std::optional<Token>
{
    std::string tokenStr{enumName(type)};
    if(_shouldTrace)
        std::cout << "Looking for a token of type: " << tokenStr << "\n";


    // Save current token
    Token currentToken = _lookahead;

    if(_lookahead.type == type)
    {
        if(_shouldTrace)
            std::cout << "Matched a token of type: " << tokenStr << " text: " << _lookahead.text << "\n";

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
            numbers.push_back(static_cast<uint16_t>(std::atoi(tok->text.c_str())));
            if(peek(TokenType::Comma))
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
    std::vector<uint16_t> ports;

    if(auto tok = match(TokenType::Ipv4Address, TokenType::Ipv6Address))
        address = tok->text;

    if(match(TokenType::Port))
    {
        if(auto tok = match(TokenType::Number))
            ports.push_back(static_cast<uint16_t>(std::atoi(tok->text.c_str())));
        else if(peek(TokenType::LBrack))
            ports = numberList();
        else
           unexpectedTokenError();
    }

    if(address.empty() && ports.empty())
        throw ParseError{"Either an ip address or a port is needed."};

    return {address, std::move(ports)};
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

    const auto &[address, ports] = addressAndPorts();

    pConditions->sourceIp = address;
    pConditions->sourcePorts = std::move(ports);
}

void Parser::destCondition(FilterConditions *pConditions)
{
    const auto &[address, ports] = addressAndPorts();

    pConditions->destIp = address;
    pConditions->destPorts = std::move(ports);
}

FilterConditions Parser::conditions()
{
    if(match(TokenType::All))
        return NoFilterConditions;

    FilterConditions filterConditions{};

    if(auto tok = match(TokenType::Inet4, TokenType::Inet6))
    {
        if(tok->type == TokenType::Inet4)
            filterConditions.ipVersion = FilterConditions::Inet4;
        else
            filterConditions.ipVersion = FilterConditions::Inet6;
    }
    if(match(TokenType::From))
        sourceCondition(&filterConditions);
    if(match(TokenType::To))
        destCondition(&filterConditions);

    return filterConditions;
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
        unexpectedTokenError();

    if(match(TokenType::OutDir))
        // cannot actually decice the layer until the "conditions" have been parsed
        // and we know wther it's ipv4 or ipv6 or both (i.e inet vs inet6)
        // instead what this does is constrains the layers that are available to the conditions
        // so if it's 'out' then we only get AUTH_CONNECT_V* NOT the AUTH_RECV_V*
        layer = FilterNode::AUTH_CONNECT_V4;
    else if(match(TokenType::InDir))
        layer = FilterNode::AUTH_CONNECT_V4;
    else
        unexpectedTokenError();

    FilterConditions filterConditions = conditions();

    return std::make_unique<FilterNode>(action, layer, std::move(filterConditions));
}

std::unique_ptr<RulesetNode> Parser::parse()
{
    auto ruleset = std::make_unique<RulesetNode>();

    try
    {
        // Get initial token
        consume();

        while(!peek(TokenType::EndOfInput))
        {
            if(peek(TokenType::PermitAction, TokenType::BlockAction))
            {
                ruleset->addChild(filter());
            }
            else
            {
                unexpectedTokenError();
            }
        }

        return ruleset;
    }
    catch(const std::exception &ex)
    {
        std::cerr << "Failed to parse: " << ex.what();
        return ruleset;
    }
    catch(...)
    {
        std::cerr << "Failed to parse: Unknown error";
        return ruleset;
    }
}
}
