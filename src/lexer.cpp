#include "lexer.hpp"
#include "driver_options.hpp"
#include "llvm/Support/raw_ostream.h"

using namespace li1I;
using std::string;
using std::istream;
using std::ostream;
using std::endl;

LexError::LexError (const TokenLocation &loc, std::string expected, istream &in)
{
    std::stringstream ss;
    ss << endl << "Lex error: expected " << expected << endl;

    std::streampos pos = loc.file_pos;
    in.seekg(loc.file_pos);
    while (in.peek() != '\n')
    {
        in.seekg(-1, std::ios_base::cur);
    }
    in.get();
    std::streamoff token_start = pos-in.tellg();

    char line[256];
    in.getline(line, 256);
    ss << "At location " << loc.line << ":" << loc.column << endl;
    ss << line << endl;
    string carat (token_start+1, ' ');
    carat += '^';
    ss << carat;
    m_message = ss.str();
}

string li1I::tokenTagToString (TokenTag tag)
{
    string s;
    switch (tag)
    {
    case TokenTag::NUM: s = "NUM"; break;
    case TokenTag::VID: s = "VID"; break;
    case TokenTag::FID: s = "FID"; break;
    case TokenTag::PROGRAM: s = "PROGRAM"; break;
    case TokenTag::LBRACE: s = "LBRACE"; break;
    case TokenTag::RBRACE: s = "RBRACE"; break;
    case TokenTag::VAR: s = "VAR"; break;
    case TokenTag::ASSIGN: s = "ASSIGN"; break;
    case TokenTag::LPAREN: s = "LPAREN"; break;
    case TokenTag::RPAREN: s = "RPAREN"; break;
    case TokenTag::FUNCTION: s = "FUNCTION"; break;
    case TokenTag::SEMI: s = "SEMI"; break;
    case TokenTag::PLUS: s = "PLUS"; break;
    case TokenTag::MINUS: s = "MINUS"; break;
    case TokenTag::TIMES: s = "TIMES"; break;
    case TokenTag::DIV: s = "DIV"; break;
    case TokenTag::EXP: s = "EXP"; break;
    case TokenTag::GT: s = "GT"; break;
    case TokenTag::LT: s = "LT"; break;
    case TokenTag::EQ: s = "EQ"; break;
    case TokenTag::NEQ: s = "NEQ"; break;
    case TokenTag::IF: s = "IF"; break;
    case TokenTag::ELSE: s = "ELSE"; break;
    case TokenTag::END: s = "END"; break;
    }

    return s;
}

void Token::print (llvm::raw_ostream &out) const
{
    out << tokenTagToString(m_token);

    switch (m_token)
    {
    case TokenTag::NUM:
        out << '(' << *m_int_data << ')'; break;
    case TokenTag::VID:
    case TokenTag::FID:
        out << '(' << *m_string_data << ')'; break;
    default: break;
    }

    out << '\n';
}

char Lexer::getChar ()
{
    char c = m_in.get();
    m_location.column += 1;
    return c;
}

bool Lexer::isCharValid (char c)
{
    switch (c)
    {
    case 'i':
    case 'I':
    case '1':
    case 'l': return true;
    default: return false;
    }
}

Token *Lexer::lexId (TokenTag token)
{
    char c;
    string *name = new string();
    while (!isspace((c = getChar())))
    {
        if (!isCharValid(c))
        {
            throw LexError(m_start_location, "identifier", m_in);
        }

        (*name) += c;
    }

    Token *t = new Token(token, m_start_location, name);
    return t;
}

Token *Lexer::lexVid ()
{
    Token *t = lexId(TokenTag::VID);
    return t;
}

Token *Lexer::lexFid ()
{
    Token *t = lexId(TokenTag::FID);
    return t;
}

Token *Lexer::lexNum ()
{
    getChar();

    char c;
    int *num = new int(0);

    while (!isspace((c = getChar())))
    {
        if (c != '1')
        {
            throw LexError(m_start_location, "more 1s", m_in);
        }

        (*num)++;
    }

    Token *t = new Token(TokenTag::NUM, m_start_location, num);
    return t;
}

Token *Lexer::lexKeyword ()
{
    TokenTag tt;

    getChar();
    switch (getChar())
    {
    case 'i':
        switch (getChar())
        {
        case 'I':
            switch (getChar())
            {
            case '1': tt = TokenTag::VAR; goto end;
            }
        case '1':
            switch (getChar())
            {
            case 'I': tt = TokenTag::PROGRAM; goto end;
            case 'l': tt = TokenTag::LPAREN; goto end;
            }
        case 'l':
            switch (getChar())
            {
            case '1': tt = TokenTag::RPAREN; goto end;
            }
        case 'i':
            switch (getChar())
            {
            case 'l': tt = TokenTag::TIMES; goto end;
            case 'i': tt = TokenTag::EXP; goto end;
            }
        }
    case 'I':
        switch (getChar())
        {
        case 'i':
            switch (getChar())
            {
            case '1': tt = TokenTag::ASSIGN; goto end;
            }
        case '1':
            switch (getChar())
            {
            case 'i': tt = TokenTag::FUNCTION; goto end;
            }
        }
    case '1':
        switch (getChar())
        {
        case 'i':
            switch (getChar())
            {
            case 'I': tt = TokenTag::LBRACE; goto end;
            case '1': tt = TokenTag::IF; goto end;
            case 'l': tt = TokenTag::ELSE; goto end;
            case 'i': tt = TokenTag::SEMI; goto end;
            }

        case 'I':
            switch (getChar())
            {
            case 'i': tt = TokenTag::RBRACE; goto end;
            }
        case '1':
            switch (getChar())
            {
            case '1': tt = TokenTag::NEQ; goto end;
            }
        }

    case 'l':
        switch (getChar())
        {
        case 'l':
            switch (getChar())
            {
            case 'i': tt = TokenTag::PLUS; goto end;
            }
        case 'i':
            switch (getChar())
            {
            case 'i': tt = TokenTag::MINUS; goto end;
            case 'l': tt = TokenTag::DIV; goto end;
            }
        case '1':
            switch (getChar())
            {
            case 'i': tt = TokenTag::GT; goto end;
            case '1': tt = TokenTag::EQ; goto end;
            case 'I': tt = TokenTag::LT; goto end;
            }
        }
        }

end:
    Token *t = new Token(tt, m_start_location);
    return t;
}

void Lexer::eatWhitespace()
{
    while (isspace(m_in.peek()))
    {
        if (getChar() == '\n')
        {
            m_location.column = 0;
            m_location.line += 1;
        }
    }
}

std::unique_ptr<const Token> Lexer::lex ()
{
    m_location.file_pos = m_in.tellg();
    m_start_location = m_location;
    if (!m_cache.empty())
    {
        std::unique_ptr<const Token> t (std::move(m_cache.front()));
        m_cache.pop();
        return t;
    }

    eatWhitespace();

    Token *t;

    if (m_in.eof())
    {
        t = new Token(TokenTag::END, m_start_location);
    }
    else
    {
        switch (m_in.peek())
        {
        case 'i': t = lexVid(); break;
        case 'I': t = lexFid(); break;
        case '1': t = lexNum(); break;
        case 'l': t = lexKeyword(); break;
        default: throw LexError(m_start_location, "valid chars", m_in);
        }
    }

    if (options::opts->getLastArg(options::OPT_emit_tokens))
    {
        t->print(llvm::outs());
    }
     
    return std::unique_ptr<const Token>(t);
}

const Token &Lexer::peekLex ()
{
    m_cache.push(lex());
    return *m_cache.front();
}
