#include <parser/parser.h>
#include <gtest/gtest.h>
#include <ranges>

namespace views = std::ranges::views;
using namespace wfpk;

TEST(ParserTests, TestBasicParsingSingleRule)
{
    std::string input = R"(permit out all)";

    Parser parser{input};

    auto tree = parser.parseTrace();
    const auto rule = static_cast<FilterNode*>(tree->children().front().get());
    ASSERT_EQ(tree->children().size(), 1);

    ASSERT_EQ(rule->action(), FilterNode::Action::Permit);
    ASSERT_EQ(rule->layer(), FilterNode::Layer::AUTH_CONNECT_V4);
    ASSERT_EQ(rule->filterConditions(), NoFilterConditions);

    std::cout << *tree;
}

TEST(ParserTests, TestBasicParsingMultipleRules)
{
    std::string input = R"(permit out all
                           block in all
                           permit in all)";

    Parser parser{input};

    auto tree = parser.parse();
    const auto rule1 = static_cast<FilterNode*>(tree->children()[0].get());
    const auto rule2 = static_cast<FilterNode*>(tree->children()[1].get());
    const auto rule3 = static_cast<FilterNode*>(tree->children()[2].get());
    ASSERT_EQ(tree->children().size(), 3);

    ASSERT_EQ(rule1->action(), FilterNode::Action::Permit);
    ASSERT_EQ(rule1->layer(), FilterNode::Layer::AUTH_CONNECT_V4);
    ASSERT_EQ(rule1->filterConditions(), NoFilterConditions);

    ASSERT_EQ(rule2->action(), FilterNode::Action::Block);
    ASSERT_EQ(rule2->layer(), FilterNode::Layer::AUTH_RECV_V4);
    ASSERT_EQ(rule2->filterConditions(), NoFilterConditions);

    ASSERT_EQ(rule3->action(), FilterNode::Action::Permit);
    ASSERT_EQ(rule3->layer(), FilterNode::Layer::AUTH_RECV_V4);
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
    ASSERT_EQ(rule->layer(), FilterNode::Layer::AUTH_CONNECT_V4);
    ASSERT_EQ(conditions.destIps, (std::vector<std::string>{"192.168.0.0/16", "10.0.0.0/8"}));
}

TEST(ParserTests, TestSourceIpList)
{
    std::string input = R"(permit out from {192.168.0.0/16, 10.0.0.0/8})";

    auto tree = Parser{input}.parse();
    ASSERT_EQ(tree->children().size(), 1);

    const auto rule = static_cast<FilterNode*>(tree->children()[0].get());
    const auto conditions = rule->filterConditions();

    ASSERT_EQ(rule->action(), FilterNode::Action::Permit);
    ASSERT_EQ(rule->layer(), FilterNode::Layer::AUTH_CONNECT_V4);
    ASSERT_EQ(conditions.sourceIps, (std::vector<std::string>{"192.168.0.0/16", "10.0.0.0/8"}));
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
