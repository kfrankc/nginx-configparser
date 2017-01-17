#include "gtest/gtest.h"
#include "config_parser.h"

// foo bar;
TEST(NginxConfigTest, ToString) {
    NginxConfigStatement statement;
    statement.tokens_.push_back("foo");
    statement.tokens_.push_back("bar");
    EXPECT_EQ(statement.ToString(0), "foo bar;\n");
}

class NginxStringConfigTest : public ::testing::Test {
	protected:
    bool ParseString(const std::string config_string) {
        std::stringstream config_stream(config_string);
        return parser_.Parse(&config_stream, &out_config_);
    }
    NginxConfigParser parser_;
    NginxConfig out_config_;
};

TEST_F(NginxStringConfigTest, BasicErrorConfig) {
    // From examples in class
    EXPECT_FALSE(ParseString("")) << "Error: failed to detect empty as invalid";
}

TEST_F(NginxStringConfigTest, BalancedCurlyBraceConfig) {
    EXPECT_FALSE(ParseString("server {listen 80;")) << "Error: failed to detect missing '}'";
    EXPECT_FALSE(ParseString("listen 80;}")) << "Error: failed to detect missing '{'";
}

TEST_F(NginxStringConfigTest, SemiColonConfig) {
    EXPECT_FALSE(ParseString("foo bar")) << "Error: failed to detect missing semicolon.";
    EXPECT_FALSE(ParseString("server {listen 80}")) << "Error: failed to detect missing semicolon within braces";
}

TEST_F(NginxStringConfigTest, TokenConfig) {
    // From examples in class
    EXPECT_TRUE(ParseString("foo bar;")) << "Error: failed to correctly parse valid string";
    EXPECT_EQ(1, out_config_.statements_.size()) << "Error: tokens parsed into incorrect number of statements";
    EXPECT_EQ("foo", out_config_.statements_.at(0)->tokens_.at(0)) << "Error: incorrectly tokenized statement";
}

TEST_F(NginxStringConfigTest, OneLineTwoStatementConfig) {
    EXPECT_TRUE(ParseString("foo bar; baz bop;")) << "Error: failed to detect one line, two statements";
}

TEST_F(NginxStringConfigTest, NestedConfig) {
    // Tests nested cases
    EXPECT_FALSE(ParseString("server {\nserver{listen 80;}")) << "Error: failed to detect error in nested braces";
    EXPECT_TRUE(ParseString("#test\n\
                              server {\n\
                                server {listen 80;}\n\
                                     }")) 
      << "Error: failed to parse doubly nested braces + comment correctly";
    EXPECT_TRUE(ParseString("server {\n\
                                server {\n\
                                  server {listen 80;}\n\
                                    foo bar;\n\
                                          }\n\
                                        }")) 
      << "Error, failed to parse complex expression correctly";
}

TEST_F(NginxStringConfigTest, CommentsConfig) {
    // Test comment cases
    EXPECT_FALSE(ParseString("#comment")) << "Error: failed to detect that just comments is invalid";
    EXPECT_TRUE(ParseString("foo; # this is a comment }{;;\nbar;")) << "Error: failed to detect this comment";
}

TEST_F(NginxStringConfigTest, QuotesConfig) {
    // Test quotations
    EXPECT_TRUE(ParseString("log_format   main '$remote_addr - $remote_user [$time_local]  $status '\
                            '\"$request\" $body_bytes_sent \"$http_referer\" '\
                            '\"$http_user_agent\" \"$http_x_forwarded_for\"';")) 
      << "Error: failed to detect valid double quotes";
    EXPECT_TRUE(ParseString("foo = \"Hello, Frank!\";")) << "Error: failed to detect valid double quotes";
}
