#pragma once

#include <string>
#include <exception>
#include <sstream>
#include <queue>

#include "ast.hpp"

namespace li1I
{
    class Token;
    class TokenLocation;
    class Lexer;

    class ParseError : public std::exception
    {
    public:
        ParseError (const TokenLocation &loc, std::string expected);
        ~ParseError() throw() {}
        virtual const char* what() const throw()
        {
            return m_message.c_str();
        }
    private:
        std::string m_message;
    };

    class Parser
    {
    public:
        Program *parse (Lexer *lexer, std::istream *in, std::string program_name);
    private:
        std::queue<Token*> m_cache;
    };
}
