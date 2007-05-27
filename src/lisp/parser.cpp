//  $Id$
//
//  TuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Matthias Braun <matze@braunis.de>
//  code in this file based on lispreader from Mark Probst
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#include <sstream>
#include <stdexcept>
#include <fstream>

#include "parser.hpp"
#include "lisp.hpp"
#include "translation.hpp"
#if defined(WIN32) && !defined(__CYGWIN__)
#  define snprintf _snprintf
#endif

namespace lisp
{

    Parser::Parser()
            : m_lexer(0)
    {}

//-----------------------------------------------------------------------------

    Parser::~Parser()
    {
        delete m_lexer;
    }

//-----------------------------------------------------------------------------

    Lisp*
    Parser::parse(const std::string& filename)
    {
        std::ifstream in(filename.c_str());
        if(!in.good())
        {
            char msg[MAX_ERROR_MESSAGE_LENGTH];
            snprintf(msg, sizeof(msg), 
                     _("Couldn't open file '%s'."),
                     filename.c_str());
            throw std::runtime_error(msg);
        }
        return parse(in);
    }

//-----------------------------------------------------------------------------

    Lisp*
    Parser::parse(std::istream& m_stream)
    {
        delete m_lexer;
        m_lexer = new Lexer(m_stream);

        m_token = m_lexer->getNextToken();
        Lisp* result = new Lisp(Lisp::TYPE_CONS);
        result->m_v.m_cons.m_car = 0;
        result->m_v.m_cons.m_cdr = readList();

        delete m_lexer;
        m_lexer = 0;

        return result;
    }

//-----------------------------------------------------------------------------

    Lisp*
    Parser::read()
    {
        Lisp* result;
        switch(m_token)
        {
        case Lexer::TOKEN_EOF:
            {
                char msg[MAX_ERROR_MESSAGE_LENGTH];
                snprintf(msg, sizeof(msg), 
                         "Parse Error at line %d: Unexpected EOF.",
                         m_lexer->getLineNumber());
                throw std::runtime_error(msg);
            }
        case Lexer::TOKEN_CLOSE_PAREN:
            {
                char msg[MAX_ERROR_MESSAGE_LENGTH];
                snprintf(msg, sizeof(msg), 
                         "Parse Error at line %d: Unexpected ')'.",
                         m_lexer->getLineNumber());
                throw std::runtime_error(msg);
            }
        case Lexer::TOKEN_OPEN_PAREN:
            result = new Lisp(Lisp::TYPE_CONS);

            m_token = m_lexer->getNextToken();

            result->m_v.m_cons.m_car = read();
            result->m_v.m_cons.m_cdr = readList();

            if(m_token != Lexer::TOKEN_CLOSE_PAREN)
            {
                char msg[MAX_ERROR_MESSAGE_LENGTH];
                snprintf(msg, sizeof(msg), 
                         "Parse Error at line %d: Expected ')'.",
                         m_lexer->getLineNumber());
                throw std::runtime_error(msg);
            }
            break;
        case Lexer::TOKEN_SYMBOL:
            {
                result = new Lisp(Lisp::TYPE_SYMBOL);
                const size_t LEN = strlen(m_lexer->getString()) + 1;
                result->m_v.m_string = new char[LEN];
                memcpy(result->m_v.m_string, m_lexer->getString(), LEN);
                break;
            }
        case Lexer::TOKEN_STRING:
            {
                result = new Lisp(Lisp::TYPE_STRING);
                const size_t LEN = strlen(m_lexer->getString()) + 1;
                result->m_v.m_string = new char[LEN];
                memcpy(result->m_v.m_string, m_lexer->getString(), LEN);
                break;
            }
        case Lexer::TOKEN_INTEGER:
            result = new Lisp(Lisp::TYPE_INTEGER);
            sscanf(m_lexer->getString(), "%d", &result->m_v.m_integer);
            break;
        case Lexer::TOKEN_REAL:
            result = new Lisp(Lisp::TYPE_REAL);
            sscanf(m_lexer->getString(), "%f", &result->m_v.m_real);
            break;
        case Lexer::TOKEN_TRUE:
            result = new Lisp(Lisp::TYPE_BOOLEAN);
            result->m_v.m_boolean = true;
            break;
        case Lexer::TOKEN_FALSE:
            result = new Lisp(Lisp::TYPE_BOOLEAN);
            result->m_v.m_boolean = false;
            break;

        default:
            // this should never happen
            assert(false);
        }

        m_token = m_lexer->getNextToken();
        return result;
    }

//-----------------------------------------------------------------------------

    Lisp*
    Parser::readList()
    {
        Lisp* result = 0;

        while(m_token != Lexer::TOKEN_CLOSE_PAREN && m_token != Lexer::TOKEN_EOF)
        {
            Lisp* newlisp = new Lisp(Lisp::TYPE_CONS);
            newlisp->m_v.m_cons.m_car = read();
            newlisp->m_v.m_cons.m_cdr = result;
            result = newlisp;
        }

        return result;
    }

} // end of namespace lisp
