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
#ifndef __LISPLEXER_H__
#define __LISPLEXER_H__

namespace lisp
{

    class Lexer
    {
    public:
        enum TokenType
        {
            TOKEN_EOF,
            TOKEN_OPEN_PAREN,
            TOKEN_CLOSE_PAREN,
        TOKEN_TRANSLATION,
            TOKEN_SYMBOL,
            TOKEN_STRING,
            TOKEN_INTEGER,
            TOKEN_REAL,
            TOKEN_TRUE,
            TOKEN_FALSE
        };

        Lexer(std::istream& stream);
        ~Lexer();

        TokenType getNextToken();

        const char* getString() const
            { return m_token_string; }

        int getLineNumber() const
            { return m_line_number; }

    private:
        enum
        {
            MAX_TOKEN_LENGTH = 4096,
            LEXER_BUFFER_SIZE = 1024
        };

        inline void nextChar();

        std::istream& m_stream;
        bool m_is_eof;
        int m_line_number;
        char m_buffer[LEXER_BUFFER_SIZE+1];
        char* m_buffer_end;
        char* m_c;
        char m_token_string[MAX_TOKEN_LENGTH + 1];
        int m_token_length;
    };

} // end of namespace lisp

#endif

