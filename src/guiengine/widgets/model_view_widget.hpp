//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015 Marianne Gagnon
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



#ifndef HEADER_MODELVIEW_HPP
#define HEADER_MODELVIEW_HPP

#include <IMesh.h>

#include "graphics/irr_driver.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "utils/aligned_array.hpp"
#include "utils/leak_check.hpp"
#include "utils/ptr_vector.hpp"

class RenderInfo;

namespace GUIEngine
{
    /** \brief A model view widget.
      * \ingroup widgetsgroup
      */
    class ModelViewWidget : public IconButtonWidget
    {
        enum RotationMode
        {
            ROTATE_OFF,
            ROTATE_CONTINUOUSLY,
            ROTATE_TO
        };
        RotationMode m_rotation_mode;
        float m_rotation_speed;
        float m_rotation_target;

        PtrVector<scene::IMesh, REF> m_models;
        std::vector<core::matrix4> m_model_location;
        std::vector<std::pair<int, int> > m_model_frames;
        std::vector<bool> m_model_render_info_affected;
        std::vector<float> m_model_animation_speed;
        std::vector<std::string> m_bone_attached;
        std::unique_ptr<RenderTarget> m_render_target;
        float m_angle;

        bool m_rtt_unsupported;

        scene::ISceneNode          *m_rtt_main_node;

        scene::ICameraSceneNode    *m_camera;

        scene::ISceneNode          *m_light;

        RenderInfo                 *m_render_info;

    public:

        LEAK_CHECK()

        ModelViewWidget();
        virtual ~ModelViewWidget();

        void add();
        void clearModels();
        void addModel(irr::scene::IMesh* mesh,
                      const core::matrix4& location = core::matrix4(),
                      const int start_loop_frame=-1,
                      const int end_loop_frame=-1,
                      bool all_parts_colorized = false,
                      float animation_speed = 0.0f,
                      const std::string& bone_name = std::string());

        void update(float delta);

        virtual void elementRemoved();

        /** Disables any model rotation */
        void setRotateOff();

        /** Makes the model rotate at given speed (in degrees per second) */
        void setRotateContinuously(float speed);

        /** Rotate to 'targetAngle' in degrees at given speed (in degrees per second) */
        void setRotateTo(float targetAngle, float speed);

        /** Returns information if currently kart is rotating */
        bool isRotating();

        void clearRttProvider();

        void setupRTTScene();

        void drawRTTScene(const irr::core::rect<s32>& dest_rect) const;

        RenderInfo* getModelViewRenderInfo() { return m_render_info; }
    };

}

#endif
