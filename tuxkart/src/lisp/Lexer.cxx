//  $Id: Lexer.cxx,v 1.1 2004/08/24 19:33:11 matzebraun Exp $
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

#include "Lexer.h"

namespace lisp
{

class EOFException
{
};

Lexer::Lexer(std::istream& newstream)
    : stream(newstream), isEof(false)
{
    try {
        // trigger a refill of the buffer
        c = 0;
        bufend = c + 1;        
        nextChar();
    } catch(EOFException& e) {
    }
}

Lexer::~Lexer()
{
}

void
Lexer::nextChar()
{
    ++c;
    if(c >= bufend) {
        if(isEof)
            throw EOFException();
        std::streamsize n = stream.readsome(buffer, BUFFER_SIZE);

        c = buffer;
        bufend = buffer + n;

        // the following is a hack that appends an additional ' ' at the end of
        // the file to avoid problems when parsing symbols/elements and a sudden
        // EOF. This is faster than relying on unget and IMO also nicer.
        if(n < BUFFER_SIZE || n == 0) {
            *bufend = ' ';
            ++bufend;
            isEof = true;
        }
    }
}

Lexer::TokenType
Lexer::getNextToken()
{
    static const char* delims = "\"();";

    try {
        while(isspace(*c)) {
            nextChar();
            if(*c == '\n')
                ++linenumber;
        };

        token_length = 0;
        
        switch(*c) {
            case ';': // comment
                while(!stream.eof()) {
                    nextChar();
                    if(*c == '\n') {
                        ++linenumber;
                        break;
                    }
                }
                return getNextToken(); // and again
            case '(':
                nextChar();
                return TOKEN_OPEN_PAREN;
            case ')':
                nextChar();
                return TOKEN_CLOSE_PAREN;
            case '"': {  // string
                int startline = linenumber;
                try {
                    while(1) {
                        if(stream.eof()) {
                            std::stringstream msg;
                            msg << "Parse Error in line " << startline << ": "
                                << "Couldn't find end of string.";
                            throw std::runtime_error(msg.str());
                        }
                        nextChar();
                        if(*c == '"')
                            break;
                    
                        if(*c == '\\') {
                            nextChar();
                            switch(*c) {
                                case 'n':
                                    *c = '\n';
                                    break;
                                case 't':
                                    *c = '\t';
                                    break;
                            }
                        }
                        if(token_length < MAX_TOKEN_LENGTH)
                            token_string[token_length++] = *c;
                    }
                    token_string[token_length] = 0;
                } catch(EOFException& ) {
                    std::stringstream msg;
                    msg << "Parse error in line " << startline << ": "
                        << "EOF while parsing string.";
                    throw std::runtime_error(msg.str());
                }
                nextChar();
                return TOKEN_STRING;
            }
            case '#': // constant
                try {
                    nextChar();

                    while(isalnum(*c) || *c == '_') {
                        if(token_length < MAX_TOKEN_LENGTH)
                            token_string[token_length++] = *c;
                        nextChar();
                    }
                    token_string[token_length] = 0;
                } catch(EOFException& ) {
                    std::stringstream msg;
                    msg << "Parse Error in line " << linenumber << ": "
                        << "EOF while parsing constant.";
                    throw std::runtime_error(msg.str());
                }

                if(strcmp(token_string, "t") == 0)
                    return TOKEN_TRUE;
                if(strcmp(token_string, "f") == 0)
                    return TOKEN_FALSE;

                // this would be the place to add more sophisticated handling of
                // constants

                {
                    std::stringstream msg;
                    msg << "Parse Error in line " << linenumber << ": "
                        << "Unknown constant '" << token_string << "'.";
                    throw std::runtime_error(msg.str());
                }

            default:
                if(isdigit(*c) || *c == '-') {
                    bool have_nondigits = false;
                    bool have_digits = false;
                    int have_floating_point = 0;

                    do {
                        if(isdigit(*c))
                            have_digits = true;
                        else if(*c == '.')
                            ++have_floating_point;
                        else if(isalnum(*c) || *c == '_')
                            have_nondigits = true;  

                        if(token_length < MAX_TOKEN_LENGTH)
                            token_string[token_length++] = *c;

                        nextChar();
                    } while(!isspace(*c) && !strchr(delims, *c));

                    token_string[token_length] = 0;

                    // no nextChar

                    if(have_nondigits || !have_digits || have_floating_point > 1)
                        return TOKEN_SYMBOL;
                    else if(have_floating_point == 1)
                        return TOKEN_REAL;
                    else
                        return TOKEN_INTEGER;
                } else {
                    do {
                        if(token_length < MAX_TOKEN_LENGTH)
                            token_string[token_length++] = *c;
                        nextChar();
                    } while(!isspace(*c) && !strchr(delims, *c));
                    token_string[token_length] = 0;
                    
                    // no nextChar

                    return TOKEN_SYMBOL;
                }       
        }
    } catch(EOFException& ) {
        return TOKEN_EOF;
    }
}

} // end of namespace lisp

