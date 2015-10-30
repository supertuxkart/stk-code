//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2015 SuperTuxKart-Team
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


/*
#include "graphics/solid_first_pass.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/stk_scene_manager.hpp"
#include "utils/profiler.hpp"


void GL3SolidFirstPass::render(const DrawCalls& draw_calls, irr::core::vector3df wind_dir)
{
    ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_SOLID_PASS1));
    irr_driver->setPhase(SOLID_NORMAL_AND_DEPTH_PASS);

    draw_calls.renderImmediateDrawList(); 
    
    
}

void IndirectInstancedSolidFirstPass::render(const DrawCalls& draw_calls, irr::core::vector3df wind_dir)
{
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, SolidPassCmd::getInstance()->drawindirectcmd);

    ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_SOLID_PASS1));
    irr_driver->setPhase(SOLID_NORMAL_AND_DEPTH_PASS);

    draw_calls.renderImmediateDrawList(); 
 
    
    
    
}

void AZDOSolidFirstPass::render(const DrawCalls& draw_calls, irr::core::vector3df wind_dir)
{
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, SolidPassCmd::getInstance()->drawindirectcmd);
    
     ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_SOLID_PASS1));
    irr_driver->setPhase(SOLID_NORMAL_AND_DEPTH_PASS);

    draw_calls.renderImmediateDrawList(); 
  
    
    
}
*/
