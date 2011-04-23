//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 Marianne Gagnon
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

#ifndef HEADER_SFX_BUFFER_HPP
#define HEADER_SFX_BUFFER_HPP


#ifdef __APPLE__
#  include <OpenAL/al.h>
#else
#  include <AL/al.h>
#endif

#include "utils/no_copy.hpp"
#include "utils/vec3.hpp"

#include <string>

class SFXBase;
class XMLNode;

/**
 * \brief The buffer (data) for one kind of sound effects
 * \ingroup audio
 */
class SFXBuffer
{
private:
    
    /** Whether the contents of the file was loaded */
    bool m_loaded;
    
    /** The file that contains the OGG audio data */
    std::string m_file;
    
    ALuint   m_buffer;
    bool     m_positional;
    float    m_rolloff;
    float    m_gain;
    
    bool loadVorbisBuffer(const std::string &name, ALuint buffer);
    
public:
    
    SFXBuffer(const std::string& file,
              bool  positional,
              float rolloff,
              float gain);
    
    SFXBuffer(const std::string& file,
              const XMLNode* node);
    
    ~SFXBuffer()
    {
    }
    
    /**
      * \brief load the buffer from file into OpenAL.
      * \note If this buffer is already loaded, this call does nothing and returns false
      * \return whether loading was successful
      */
    bool     load();
    
    /**
      * \brief Frees the loaded buffer
      * Cannot appear in destructor because copy-constructors may be used,
      * and the OpenAL source must not be deleted on a copy
      */
    void     unload();
    
    /** \return whether this buffer was loaded from disk */
    bool     isLoaded()       const { return m_loaded; }
    
    /** Only returns a valid buffer if isLoaded() returned true */
    ALuint   getBuffer()      const { return m_buffer; }
    
    bool     isPositional()   const { return m_positional; }
    float    getRolloff()     const { return m_rolloff; }
    float    getGain()        const { return m_gain; }
    std::string getFileName() const { return m_file; }
    
    void     setPositional(bool positional) { m_positional = positional; }


};


#endif // HEADER_SFX_BUFFER_HPP

