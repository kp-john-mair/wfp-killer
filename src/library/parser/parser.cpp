#include <parser/parser.h>

namespace wfpk {
auto Parser::match(TokenType type) -> std::optional<Token>
{
    if(_shouldTrace)
        std::cout << "Looking for a token of type: " << enumName(type) << "\n";

    // Save current token
    Token currentToken = _lookahead;

    if(peek().type == type)
    {
        if(_shouldTrace)
            std::cout << "Matched a token of type: " << currentToken << "\n";

        // Move the input to the next token
        consume();

        return currentToken;
    }
    else
    {
        return {};
    }
}

auto Parser::numberList() -> std::vector<uint16_t>
{
    auto results = list([](Token tok)
    {
        return static_cast<uint16_t>(std::atoi(tok.text.c_str()));
    }, TokenType::Number);

    return results;
}

auto Parser::ipList() -> std::vector<std::string>
{
    auto results = list([](Token tok)
    {
        return tok.text;
    }, TokenType::Ipv4Address, TokenType::Ipv6Address);

    return results;
}

// Does not return a list - only returns one protocol type.
// But the protocols can be written as a list in the grammar,
// i.e { tcp, udp } which is a list - maps to the AllTransports enum value.
auto Parser::transportProtocolList()
    -> FilterConditions::TransportProtocol
{
    using TransportProtocol = FilterConditions::TransportProtocol;

    auto results = list([](Token tok)
    {
        return (tok.type == TokenType::TcpTransport ? TransportProtocol::Tcp : TransportProtocol::Udp);
    }, TokenType::TcpTransport, TokenType::UdpTransport);

    // Allow at most 2 values in list
    if(results.size() > 2)
        throw ParseError(std::format("Expected at most 2 values in transport protocol list, but got: {} @ {}", results.size(), sourceLocation().toString()));

    if(std::ranges::all_of(results, [](auto val) { return val == TransportProtocol::Tcp; }))
        return TransportProtocol::Tcp;
    else if(std::ranges::all_of(results, [](auto val) { return val == TransportProtocol::Udp; }))
        return TransportProtocol::Udp;
    // If both Tcp and Udp appear in the list, then return 'AllTransports' as we
    // match all supported transport protocols
    else
        return TransportProtocol::AllTransports;
}

auto Parser::transportProtocol()
    -> FilterConditions::TransportProtocol
{
    using TransportProtocol = FilterConditions::TransportProtocol;

    std::vector<FilterConditions::TransportProtocol> protList;
    if(auto tok = match(TokenType::TcpTransport, TokenType::UdpTransport))
    {
        return (tok->type == TokenType::TcpTransport ? TransportProtocol::Tcp :
            TransportProtocol::Udp);
    }
    else if(peek(TokenType::LBrack))
        return transportProtocolList();
    else
        unexpectedTokenError();

    return TransportProtocol::AllTransports;;
}

auto Parser::addressAndPorts()
    -> std::pair<std::vector<std::string>, std::vector<uint16_t>>
{
    std::vector<std::string> addresses;
    std::vector<uint16_t> ports;

    if(auto tok = match(TokenType::Ipv4Address, TokenType::Ipv6Address))
        addresses.push_back(tok->text);
    else if(peek(TokenType::LBrack))
        addresses = ipList();

    if(match(TokenType::Port))
    {
        if(auto tok = match(TokenType::Number))
            ports.push_back(static_cast<uint16_t>(std::atoi(tok->text.c_str())));
        else if(peek(TokenType::LBrack))
            ports = numberList();
        else
           unexpectedTokenError();
    }

    if(addresses.empty() && ports.empty())
        throw ParseError{std::format("Either an ip address or a port is needed, @ {}", sourceLocation().toString())};

    return {addresses, std::move(ports)};
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

    const auto &[addresses, ports] = addressAndPorts();

    pConditions->sourceIps = addresses;
    pConditions->sourcePorts = std::move(ports);
}

void Parser::destCondition(FilterConditions *pConditions)
{
    const auto &[addresses, ports] = addressAndPorts();

    pConditions->destIps = addresses;
    pConditions->destPorts = std::move(ports);
}

FilterConditions Parser::conditions()
{
    using IpVersion = FilterConditions::IpVersion;

    if(match(TokenType::All))
        return NoFilterConditions;

    FilterConditions filterConditions{};

    if(auto tok = match(TokenType::Inet4, TokenType::Inet6))
    {
        if(tok->type == TokenType::Inet4)
            filterConditions.ipVersion = IpVersion::Inet4;
        else
            filterConditions.ipVersion = IpVersion::Inet6;
    }
    if(match(TokenType::Proto))
    {
        filterConditions.transportProtocol = transportProtocol();
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
        // cannot actually decide the layer until the "conditions" have been parsed
        // and we know wther it's ipv4 or ipv6 or both (i.e inet vs inet6)
        // instead what this does is constrains the layers that are available to the conditions
        // so if it's 'out' then we only get AUTH_CONNECT_V* NOT the AUTH_RECV_V*
        layer = FilterNode::AUTH_CONNECT_V4;
    else if(match(TokenType::InDir))
        layer = FilterNode::AUTH_RECV_V4;
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
                ruleset->addChild(filter());
            else
                unexpectedTokenError();
        }

        return ruleset;
    }
    catch(const std::exception &ex)
    {
        std::cerr << "Failed to parse: " << ex.what();
        return {};
    }
    catch(...)
    {
        std::cerr << "Failed to parse: Unknown error";
        return {};
    }
}
}
