#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <queue>
#include <exception>
#include <memory>

namespace llvm
{
    class raw_ostream;
}


namespace li1I
{
    enum class TokenTag
    {
        NUM,
        VID,
        FID,
        PROGRAM,
        LBRACE,
        RBRACE,
        VAR,
        ASSIGN,
        LPAREN,
        RPAREN,
        FUNCTION,
        SEMI,
        PLUS,
        MINUS,
        TIMES,
        DIV,
        EXP,
        GT,
        LT,
        EQ,
        NEQ,
        IF,
        ELSE,
        END
    };

    std::string tokenTagToString (TokenTag tag);

    struct TokenLocation
    {
        std::streampos file_pos;
        unsigned int line;
        unsigned int column;
    };
    
    class Token
    {
    public:
        Token(TokenTag token, TokenLocation loc) : m_token(token), m_location(loc) {}
        Token(TokenTag token, TokenLocation loc, std::string *string_data) 
            : m_token(token), m_location(loc), m_string_data(string_data) {}
        Token(TokenTag token, TokenLocation loc, int *int_data) 
            : m_token(token), m_location(loc), m_int_data(int_data) {}

        Token(const Token&) =delete;
        Token &operator=(const Token&) =delete;

        ~Token()
        {
            switch (m_token)
            {
            case TokenTag::FID:
            case TokenTag::VID: m_string_data.~unique_ptr<std::string>(); break;
            case TokenTag::NUM: m_int_data.~unique_ptr<int>(); break;
            default: break;
            }
        }


        void print(llvm::raw_ostream &out) const;
        inline const TokenTag &token() const { return m_token; }
        inline const TokenLocation &location() const { return m_location; }
        inline const std::string &string_data() const { return *m_string_data; }
        inline const int &int_data() const { return *m_int_data; }

    private:
        TokenTag m_token;
        TokenLocation m_location;

        union
        {
            std::unique_ptr<std::string> m_string_data;
            std::unique_ptr<int> m_int_data;
        };
    };

    class LexError : public std::exception
    {
    public:
        LexError (const TokenLocation &loc, std::string expected, std::istream &in);
        ~LexError() throw() {}
        virtual const char* what() const throw()
        {
            return m_message.c_str();
        }
    private:
        std::string m_message;
    };

    class Lexer
    {
    public:
        Lexer (std::istream &in) : m_in(in), m_buf(), m_cache(), m_location(), m_start_location() {}
        std::unique_ptr<const Token> lex();
        const Token &peekLex();

    private:
        char getChar ();
        bool isCharValid (char c);
        Token *lexId (TokenTag token);
        Token *lexVid ();
        Token *lexFid ();
        Token *lexNum ();
        Token *lexKeyword ();
        void eatWhitespace();

        std::istream &m_in;
        std::stringstream m_buf;
        std::queue<std::unique_ptr<const Token> > m_cache;
        TokenLocation m_location;
        TokenLocation m_start_location;
    };
}
