//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009 Marianne Gagnon
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

#include "graphics/irr_driver.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/widgets/model_view_widget.hpp"
using namespace GUIEngine;
using namespace irr::core;
using namespace irr::gui;

ModelViewWidget::ModelViewWidget()
{
    m_type = WTYPE_MODEL_VIEW;
    m_rtt_provider = NULL;
    m_rotation_mode = ROTATE_OFF;
}
// -----------------------------------------------------------------------------
ModelViewWidget::~ModelViewWidget()
{
    GUIEngine::needsUpdate.remove(this);
    
    delete m_rtt_provider;
    m_rtt_provider = NULL;
}
// -----------------------------------------------------------------------------
void ModelViewWidget::add()
{
    rect<s32> widget_size = rect<s32>(x, y, x + w, y + h);
    //stringw& message = m_text;
    
    IGUIImage* btn = GUIEngine::getGUIEnv()->addImage(widget_size, m_parent, getNewNoFocusID());
    m_element = btn;
    btn->setUseAlphaChannel(true);
    btn->setTabStop(false);
    btn->setScaleImage(true);
    
    //m_element = GUIEngine::getGUIEnv()->addMeshViewer(widget_size, NULL, ++id_counter_2);
    
    /*
     TODO: remove this unclean thing, I think irrlicht provides this feature:
        virtual void IGUIElement::OnPostRender (u32 timeMs)
        \brief animate the element and its children. 
     */
    GUIEngine::needsUpdate.push_back(this);
    
    angle = 0;
    
    id = m_element->getID();
    //m_element->setTabOrder(id);
    m_element->setTabGroup(false);
    m_element->setTabStop(false);
}   // add

// -----------------------------------------------------------------------------
void ModelViewWidget::clearModels()
{
    m_models.clearWithoutDeleting();
    m_model_location.clear();
    
    delete m_rtt_provider;
    m_rtt_provider = NULL;
}
// -----------------------------------------------------------------------------
void ModelViewWidget::addModel(irr::scene::IMesh* mesh, const Vec3& location)
{
    m_models.push_back(mesh);
    m_model_location.push_back(location);
    
    /*
     ((IGUIMeshViewer*)m_element)->setMesh( mesh );
     
     video::SMaterial mat = mesh->getMeshBuffer(0)->getMaterial(); //mesh_view->getMaterial();
     mat.setFlag(EMF_LIGHTING , false);
     //mat.setFlag(EMF_GOURAUD_SHADING, false);
     //mat.setFlag(EMF_NORMALIZE_NORMALS, true);
     ((IGUIMeshViewer*)m_element)->setMaterial(mat);
     */
    
    delete m_rtt_provider;
    m_rtt_provider = NULL;
}
// -----------------------------------------------------------------------------
void ModelViewWidget::update(float delta)
{
    if (m_rotation_mode == ROTATE_CONTINUOUSLY)
    {
        angle += delta*m_rotation_speed;
        if (angle > 360) angle -= 360;
    }
    else if (m_rotation_mode == ROTATE_TO)
    {
        // check if we should rotate clockwise or counter-clockwise to reach the target faster
        // (taking warp-arounds into account)
        const int angle_distance_from_end  = (int)(360 - angle);
        const int target_distance_from_end = (int)(360 - angle);

        int distance_with_positive_rotation;
        int distance_with_negative_rotation;
        
        if (angle < m_rotation_target)
        {
            distance_with_positive_rotation = (int)(m_rotation_target - angle);
            distance_with_negative_rotation = (int)(angle + target_distance_from_end);
        }
        else
        {
            distance_with_positive_rotation = (int)(angle_distance_from_end + m_rotation_target);
            distance_with_negative_rotation = (int)(angle - m_rotation_target);
        }
        
        //std::cout << "distance_with_positive_rotation=" << distance_with_positive_rotation <<
        //" distance_with_negative_rotation=" << distance_with_negative_rotation << " angle="<< angle  <<std::endl;
        
        if (distance_with_positive_rotation < distance_with_negative_rotation) 
        {
            angle += delta*(3.0f + std::min(distance_with_positive_rotation, distance_with_negative_rotation)*2.0f);
        }
        else
        {
            angle -= delta*(3.0f + std::min(distance_with_positive_rotation, distance_with_negative_rotation)*2.0f);
        }
        if (angle > 360) angle -= 360;
        if (angle < 0) angle += 360;

        // stop rotating when target reached
        if (fabsf(angle - m_rotation_target) < 2.0f) m_rotation_mode = ROTATE_OFF;
    }
    
    if (m_rtt_provider == NULL)
    {
        std::string name = "model view ";
        name += m_properties[PROP_ID].c_str();
        m_rtt_provider = new IrrDriver::RTTProvider(core::dimension2d< u32 >(512, 512), name );
        m_rtt_provider->setupRTTScene(m_models, m_model_location);
    }
    
    m_texture = m_rtt_provider->renderToTexture(angle);
    ((IGUIImage*)m_element)->setImage(m_texture);
}


void ModelViewWidget::setRotateOff()
{
    m_rotation_mode = ROTATE_OFF;
}
void ModelViewWidget::setRotateContinuously(float speed)
{
    m_rotation_mode = ROTATE_CONTINUOUSLY;
    m_rotation_speed = speed;
}
void ModelViewWidget::setRotateTo(float targetAngle, float speed)
{
    m_rotation_mode = ROTATE_TO;
    m_rotation_speed = speed;
    m_rotation_target = targetAngle;
}

