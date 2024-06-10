#include <parser/parser.h>
#include <gtest/gtest.h>
#include <ranges>

namespace views = std::ranges::views;
using namespace wfpk;
using enum TokenType;

TEST(ParserTests, TestBasicParsing)
{
    std::string input = R"(permit out inet6 proto {udp,tcp} from port {1, 2, 3, 4}
                           block in from 1.1.1.1)";
    Lexer lexer{input};

    Parser parser{lexer};

    auto tree = parser.parseTrace();

    std::cout << *tree;
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
