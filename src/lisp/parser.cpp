//  $Id$
//
//  TuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Matthias Braun <matze@braunis.de>
//  code in this file based on lispreader from Mark Probst
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
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

#include "lisp/parser.hpp"
#include "lisp/lisp.hpp"
#include "utils/translation.hpp"

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
            std::ostringstream msg;
            msg << "Couldn't open file '" << filename << "'.";
            throw std::runtime_error(msg.str());
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
        Lisp* result=NULL;
        switch(m_token)
        {
        case Lexer::TOKEN_EOF:
            {
                std::ostringstream msg;
                msg << "Parse Error at line " << m_lexer->getLineNumber()
                    << ": Unexpected EOF.";
                throw std::runtime_error(msg.str());
            }
        case Lexer::TOKEN_CLOSE_PAREN:
            {
                std::ostringstream msg;
                msg << "Parse Error at line " << m_lexer->getLineNumber()
                    << ": Unexpected ')'.";
                throw std::runtime_error(msg.str());
            }
        case Lexer::TOKEN_TRANSLATION:
           {
                result = new Lisp(Lisp::TYPE_STRING);
                m_token = m_lexer->getNextToken();
                Lisp* next=read();
                if(next->getType()!=Lisp::TYPE_STRING)
                {
                    std::ostringstream msg;
                    msg << "Parse Error at line " << m_lexer->getLineNumber()
                        << ": No string inside translation.";
                    throw std::runtime_error(msg.str());
                }
                irr::core::stringc trans=irr::core::stringc(_(next->m_v.m_string));
                const size_t LEN = trans.size() + 1;
                result->m_v.m_string = new char[LEN];
                memcpy(result->m_v.m_string, trans.c_str(), LEN);
                delete next;
                break;
           }
        case Lexer::TOKEN_OPEN_PAREN:
            result = new Lisp(Lisp::TYPE_CONS);

            m_token = m_lexer->getNextToken();

            result->m_v.m_cons.m_car = read();
            result->m_v.m_cons.m_cdr = readList();

            if(m_token != Lexer::TOKEN_CLOSE_PAREN)
            {
                std::ostringstream msg;
                msg << "Parse Error at line " << m_lexer->getLineNumber()
                    << ": Expected ')'.";
                throw std::runtime_error(msg.str());
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
