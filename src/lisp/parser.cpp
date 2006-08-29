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

namespace lisp
{

Parser::Parser()
    : lexer(0)
{
}

Parser::~Parser()
{
    delete lexer;
}

Lisp*
Parser::parse(const std::string& filename)
{
    std::ifstream in(filename.c_str());
    if(!in.good()) {
        std::stringstream msg;
        msg << "Parser problem: Couldn't open file '" << filename << "'.";
        throw std::runtime_error(msg.str());
    }
    return parse(in);
}

Lisp*
Parser::parse(std::istream& stream)
{
    delete lexer;
    lexer = new Lexer(stream);

    token = lexer->getNextToken();
    Lisp* result = new Lisp(Lisp::TYPE_CONS);
    result->v.cons.car = 0;
    result->v.cons.cdr = readList();

    delete lexer;
    lexer = 0;

    return result;    
}

Lisp*
Parser::read()
{
    Lisp* result;
    switch(token) {
        case Lexer::TOKEN_EOF: {
            std::stringstream msg;
            msg << "Parse Error at line " << lexer->getLineNumber() << ": "
                << "Unexpected EOF.";
            throw std::runtime_error(msg.str());
        }
        case Lexer::TOKEN_CLOSE_PAREN: {
            std::stringstream msg;
            msg << "Parse Error at line " << lexer->getLineNumber() << ": "
                << "Unexpected ')'.";
            throw std::runtime_error(msg.str());
        }
        case Lexer::TOKEN_OPEN_PAREN:
            result = new Lisp(Lisp::TYPE_CONS);

            token = lexer->getNextToken();
            
            result->v.cons.car = read();
            result->v.cons.cdr = readList();
            
            if(token != Lexer::TOKEN_CLOSE_PAREN) {
                std::stringstream msg;
                msg << "Parse Error at line " << lexer->getLineNumber() << ": "
                    << "Expected ')'.";
                throw std::runtime_error(msg.str());
            }
            break;
        case Lexer::TOKEN_SYMBOL: {
            result = new Lisp(Lisp::TYPE_SYMBOL);
            size_t len = strlen(lexer->getString()) + 1;
            result->v.string = new char[len];
            memcpy(result->v.string, lexer->getString(), len);
            break;
        }
        case Lexer::TOKEN_STRING: {
            result = new Lisp(Lisp::TYPE_STRING);
            size_t len = strlen(lexer->getString()) + 1;
            result->v.string = new char[len];
            memcpy(result->v.string, lexer->getString(), len);
            break;
        }
        case Lexer::TOKEN_INTEGER:
            result = new Lisp(Lisp::TYPE_INTEGER);
            sscanf(lexer->getString(), "%d", &result->v.integer);
            break;
        case Lexer::TOKEN_REAL:
            result = new Lisp(Lisp::TYPE_REAL);
            sscanf(lexer->getString(), "%f", &result->v.real);
            break;
        case Lexer::TOKEN_TRUE:
            result = new Lisp(Lisp::TYPE_BOOLEAN);
            result->v.boolean = true;
            break;
        case Lexer::TOKEN_FALSE:
            result = new Lisp(Lisp::TYPE_BOOLEAN);
            result->v.boolean = false;
            break;

        default:
            // this should never happen
            assert(false);
    }

    token = lexer->getNextToken();
    return result;
}

Lisp*
Parser::readList()
{
    Lisp* result = 0;

    while(token != Lexer::TOKEN_CLOSE_PAREN && token != Lexer::TOKEN_EOF) {
        Lisp* newlisp = new Lisp(Lisp::TYPE_CONS);
        newlisp->v.cons.car = read();
        newlisp->v.cons.cdr = result;
        result = newlisp;
    }

    return result;
}

} // end of namespace lisp
