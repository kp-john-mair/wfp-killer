#include <parser/parser.h>
#include <gtest/gtest.h>
#include <ranges>

namespace views = std::ranges::views;
using namespace wfpk;

namespace {
auto filterConditionsFor(std::string input)
    -> FilterConditions
{
    auto tree = Parser{input}.parse();
    const auto rule = static_cast<FilterNode*>(tree->children()[0].get());
    return rule->filterConditions();
}
}

TEST(ParserTests, TestBasicParsingSingleRule)
{
    std::string input = R"(permit out all)";

    Parser parser{input};

    auto tree = parser.parseTrace();
    const auto rule = static_cast<FilterNode*>(tree->children().front().get());
    ASSERT_EQ(tree->children().size(), 1);

    ASSERT_EQ(rule->action(), FilterNode::Action::Permit);
    ASSERT_EQ(rule->direction(), FilterNode::Direction::Out);
    ASSERT_EQ(rule->filterConditions(), NoFilterConditions);

    std::cout << *tree;
}

TEST(ParserTests, TestBasicParsingMultipleRules)
{
    std::string input = R"(permit out all
                           block in all
                           permit in all)";

    Parser parser{input};

    auto tree = parser.parseTrace();
    const auto rule1 = static_cast<FilterNode*>(tree->children()[0].get());
    const auto rule2 = static_cast<FilterNode*>(tree->children()[1].get());
    const auto rule3 = static_cast<FilterNode*>(tree->children()[2].get());
    ASSERT_EQ(tree->children().size(), 3);

    ASSERT_EQ(rule1->action(), FilterNode::Action::Permit);
    ASSERT_EQ(rule1->direction(), FilterNode::Direction::Out);
    ASSERT_EQ(rule1->filterConditions(), NoFilterConditions);

    ASSERT_EQ(rule2->action(), FilterNode::Action::Block);
    ASSERT_EQ(rule2->direction(), FilterNode::Direction::In);
    ASSERT_EQ(rule2->filterConditions(), NoFilterConditions);

    ASSERT_EQ(rule3->action(), FilterNode::Action::Permit);
    ASSERT_EQ(rule3->direction(), FilterNode::Direction::In);
    ASSERT_EQ(rule3->filterConditions(), NoFilterConditions);

    std::cout << *tree;
}

TEST(ParserTests, TestDestIpList)
{
    std::string input = R"(permit out to {192.168.0.0/16, 10.0.0.0/8})";

    auto tree = Parser{input}.parse();
    ASSERT_EQ(tree->children().size(), 1);

    const auto rule = static_cast<FilterNode*>(tree->children()[0].get());
    const auto conditions = rule->filterConditions();

    ASSERT_EQ(rule->action(), FilterNode::Action::Permit);
    ASSERT_EQ(rule->direction(), FilterNode::Direction::Out);
    ASSERT_EQ(conditions.destIps.v4, (std::vector<std::string>{"192.168.0.0/16", "10.0.0.0/8"}));
}

TEST(ParserTests, TestSourceIpListv4)
{
    std::string input = R"(permit out from {192.168.0.0/16, 10.0.0.0/8})";

    auto tree = Parser{input}.parse();
    ASSERT_EQ(tree->children().size(), 1);

    const auto rule = static_cast<FilterNode*>(tree->children()[0].get());
    const auto conditions = rule->filterConditions();

    ASSERT_EQ(rule->action(), FilterNode::Action::Permit);
    ASSERT_EQ(rule->direction(), FilterNode::Direction::Out);
    ASSERT_EQ(conditions.sourceIps.v4, (std::vector<std::string>{"192.168.0.0/16", "10.0.0.0/8"}));
}

TEST(ParserTests, TestSourceIpListv6)
{
    std::string input = R"(permit out from {123::1/64, 234::2/128})";

    auto tree = Parser{input}.parse();
    ASSERT_EQ(tree->children().size(), 1);

    const auto rule = static_cast<FilterNode*>(tree->children()[0].get());
    const auto conditions = rule->filterConditions();

    ASSERT_EQ(rule->action(), FilterNode::Action::Permit);
    ASSERT_EQ(rule->direction(), FilterNode::Direction::Out);
    ASSERT_EQ(conditions.sourceIps.v6, (std::vector<std::string>{"123::1/64", "234::2/128"}));
}

TEST(ParserTests, TestSourceIpListMixed)
{
    std::string input = R"(permit out from {123::1/64, 192.168.0.0/16, 234::2/128, 10.0.0.0/8})";

    auto tree = Parser{input}.parse();
    ASSERT_EQ(tree->children().size(), 1);

    const auto rule = static_cast<FilterNode*>(tree->children()[0].get());
    const auto conditions = rule->filterConditions();

    ASSERT_EQ(rule->action(), FilterNode::Action::Permit);
    ASSERT_EQ(rule->direction(), FilterNode::Direction::Out);
    ASSERT_EQ(conditions.sourceIps.v6, (std::vector<std::string>{"123::1/64", "234::2/128"}));
    ASSERT_EQ(conditions.sourceIps.v4, (std::vector<std::string>{"192.168.0.0/16", "10.0.0.0/8"}));
}

TEST(ParserTests, TestTransportProtocol)
{
    // UDP only
    auto conditions = filterConditionsFor("permit out proto udp");
    ASSERT_EQ(conditions.transportProtocol, FilterConditions::TransportProtocol::Udp);
    // TCP only
    conditions = filterConditionsFor("permit out proto tcp");
    ASSERT_EQ(conditions.transportProtocol, FilterConditions::TransportProtocol::Tcp);
    // Both UDP and TCP
    conditions = filterConditionsFor("permit out proto {tcp, udp}");
    ASSERT_EQ(conditions.transportProtocol, FilterConditions::TransportProtocol::AllTransports);
    // Both UDP
    conditions = filterConditionsFor("permit out proto {udp, udp}");
    ASSERT_EQ(conditions.transportProtocol, FilterConditions::TransportProtocol::Udp);
    // Both TCP
    conditions = filterConditionsFor("permit out proto {tcp, tcp}");
    ASSERT_EQ(conditions.transportProtocol, FilterConditions::TransportProtocol::Tcp);
}

TEST(ParserTests, TestErrorsForTransportProtocol)
{
    // Only allowed 2 elements max
    auto tree = Parser{"permit out proto {udp, tcp, udp}"}.parseTrace();
    // the unique_ptr will be null - since parse failed, instead we trace an error
    ASSERT_EQ(tree == nullptr, true);
}

TEST(ParserTests, TestErrorsIp6VersionMismatch)
{
    // An ipv6 ip is not allowed for inet (which is ipv4)
    auto tree = Parser{"permit out inet to 123::1"}.parseTrace();
    // the unique_ptr will be null - since parse failed, instead we trace an error
    ASSERT_EQ(tree == nullptr, true);
}

TEST(ParserTests, TestErrorsIp4VersionMismatch)
{
    // An ipv4 ip is not allowed for ine6 (which is ipv6)
    auto tree = Parser{"permit out inet6 to 1.1.1.1"}.parseTrace();
    // the unique_ptr will be null - since parse failed, instead we trace an error
    ASSERT_EQ(tree == nullptr, true);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
