#include <queue>
#include <memory>
#include "llvm/Support/raw_ostream.h"

#include "parser.hpp"
#include "lexer.hpp"

using namespace li1I;

using std::vector;
using std::string;
using std::istream;
using std::endl;

static Lexer *the_lexer;
static istream *lexed_file;

ParseError::ParseError (const TokenLocation &loc, std::string expected)
{
    std::stringstream ss;
    ss << endl << "Parse error: expected " << expected << endl;

    std::streampos pos = loc.file_pos;
    lexed_file->seekg(loc.file_pos);
    while (lexed_file->peek() != '\n')
    {
        lexed_file->seekg(-1, std::ios_base::cur);
    }
    lexed_file->get();
    std::streamoff token_start = pos-lexed_file->tellg();

    char line[256];
    lexed_file->getline(line, 256);
    ss << "At location " << loc.line << ":" << loc.column << endl;
    ss << line << endl;
    string carat (token_start+1, ' ');
    carat += '^';
    ss << carat;
    m_message = ss.str();
}


void mandatoryToken (TokenTag needed, const Token &found)
{
    if (needed != found.token())
    {
        throw ParseError(found.location(), tokenTagToString(needed));
    }
}

IntExpr::IntExpr()
{
    std::unique_ptr<const Token> t = the_lexer->lex();
    mandatoryToken(TokenTag::NUM, *t);
    m_value = t->int_data();
}

CallExpr::CallExpr()
{
    std::unique_ptr<const Token> t = the_lexer->lex();
    mandatoryToken(TokenTag::FID, *t);
    m_fid = t->string_data();
}

VarExpr::VarExpr()
{
    std::unique_ptr<const Token> t = the_lexer->lex();
    mandatoryToken(TokenTag::VID, *t);
    m_vid = t->string_data();
}

OpExpr::OpExpr()
{
    std::unique_ptr<const Token> t = the_lexer->lex();

    switch (t->token())
    {
    case TokenTag::PLUS: m_op = Operator::PLUS; break;
    case TokenTag::MINUS: m_op = Operator::MINUS; break;
    case TokenTag::TIMES: m_op = Operator::TIMES; break;
    case TokenTag::DIV: m_op = Operator::DIV; break;
    case TokenTag::EXP: m_op = Operator::EXP; break;
    case TokenTag::GT: m_op = Operator::GT; break;
    case TokenTag::LT: m_op = Operator::LT; break;
    case TokenTag::EQ: m_op = Operator::EQ; break;
    case TokenTag::NEQ:  m_op = Operator::NEQ; break;
    default: throw ParseError(t->location(), "operator");
    }
}

DeclExpr::DeclExpr()
{
    mandatoryToken(TokenTag::VAR, *the_lexer->lex());

    std::unique_ptr<const Token> t = the_lexer->lex();
    mandatoryToken(TokenTag::VID, *t);
    m_vid = t->string_data();

    mandatoryToken(TokenTag::ASSIGN, *the_lexer->lex());

    m_value = std::unique_ptr<RPNExpr>(new RPNExpr);
}

IfExpr::IfExpr ()
{
    mandatoryToken(TokenTag::IF, *the_lexer->lex());
    mandatoryToken(TokenTag::LPAREN, *the_lexer->lex());
    m_condition = std::unique_ptr<RPNExpr>(new RPNExpr);
    mandatoryToken(TokenTag::RPAREN, *the_lexer->lex());

    m_if_forms = std::unique_ptr<RPNExpr>(new RPNExpr);

    mandatoryToken(TokenTag::ELSE, *the_lexer->lex());

    m_else_forms = std::unique_ptr<RPNExpr>(new RPNExpr);
}

RPNExpr::RPNExpr ()
{
    while (true)
    {
        const Token &t = the_lexer->peekLex();
        if (t.token() == TokenTag::SEMI)
        {
            break;
        }

        switch (t.token())
        {
        case TokenTag::NUM: m_exprs.push_back(std::unique_ptr<IntExpr>(new IntExpr)); break;
        case TokenTag::FID: m_exprs.push_back(std::unique_ptr<CallExpr>(new CallExpr)); break;
        case TokenTag::VAR: m_exprs.push_back(std::unique_ptr<DeclExpr>(new DeclExpr)); break;
        case TokenTag::VID: m_exprs.push_back(std::unique_ptr<VarExpr>(new VarExpr)); break;
        case TokenTag::IF: m_exprs.push_back(std::unique_ptr<IfExpr>(new IfExpr)); break;
        case TokenTag::PLUS:
        case TokenTag::MINUS:
        case TokenTag::TIMES:
        case TokenTag::DIV:
        case TokenTag::EXP:
        case TokenTag::GT:
        case TokenTag::LT:
        case TokenTag::EQ:
        case TokenTag::NEQ: m_exprs.push_back(std::unique_ptr<OpExpr>(new OpExpr)); break;
        default: throw ParseError(t.location(), "expression");
        }
    } 

    std::unique_ptr<const Token> semi = the_lexer->lex();

    if (m_exprs.empty())
    {
        throw ParseError (semi->location(), "non-empty expression");
    }
}

Function::Function()
{
    mandatoryToken(TokenTag::FUNCTION, *the_lexer->lex());

    std::unique_ptr<const Token> t = the_lexer->lex();
    mandatoryToken(TokenTag::FID, *t);
    m_name = t->string_data();

    if (the_lexer->peekLex().token() == TokenTag::LPAREN)
    {
        mandatoryToken(TokenTag::LPAREN, *the_lexer->lex());
        while (the_lexer->peekLex().token() != TokenTag::RPAREN)
        {
            m_args.push_back(std::unique_ptr<VarExpr>(new VarExpr));
        }
        mandatoryToken(TokenTag::RPAREN, *the_lexer->lex());
    }

    m_expr = std::unique_ptr<RPNExpr>(new RPNExpr);
}

Program::Program(std::string name) : m_name(std::move(name))
{
    mandatoryToken(TokenTag::PROGRAM, *the_lexer->lex());
    mandatoryToken(TokenTag::LBRACE, *the_lexer->lex());

    while (the_lexer->peekLex().token() != TokenTag::RBRACE)
    {
        m_functions.push_back(std::unique_ptr<Function>(new Function));
    }

    std::unique_ptr<const Token> t = the_lexer->lex();
    mandatoryToken(TokenTag::RBRACE, *t);

    for (auto &f : m_functions)
    {
        if (f->name() == "IIII")
        {
            return;
        }
    }

    throw ParseError(t->location(), "IIII function definition");
}

Program *Parser::parse (Lexer *lexer, istream *in, std::string program_name)
{
    the_lexer = lexer;
    lexed_file = in;
    return new Program(std::move(program_name));
}
