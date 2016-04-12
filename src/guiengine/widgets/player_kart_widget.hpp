//  SuperTuxKart - a fun racing game with go-kart
//
//  Copyright (C) 2006-2015 SuperTuxKart-Team
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

#ifndef PLAYER_KART_WIDGET_HPP
#define PLAYER_KART_WIDGET_HPP

#include "guiengine/widgets/spinner_widget.hpp"
#include "states_screens/state_manager.hpp"
#include <IGUIStaticText.h>
#include <IGUIImage.h>
#include <string>


class KartSelectionScreen;
class NetworkPlayerProfile;

namespace GUIEngine
{
    class PlayerNameSpinner;
    class KartStatsWidget;
    class ModelViewWidget;
    class LabelWidget;

    /** A widget representing the kart selection for a player (i.e. the player's
     *  number, name, the kart view, the kart's name) */
    class PlayerKartWidget : public GUIEngine::Widget,
        public GUIEngine::SpinnerWidget::ISpinnerConfirmListener
    {
        /** Whether this player confirmed their selection */
        bool m_ready;
        /** If the player is handicapped. */
        bool m_handicapped;

        /** widget coordinates */
        int player_name_x, player_name_y, player_name_w, player_name_h;
        int model_x, model_y, model_w, model_h;
        int kart_name_x, kart_name_y, kart_name_w, kart_name_h;
        int m_kart_stats_x, m_kart_stats_y, m_kart_stats_w, m_kart_stats_h;

        /** A reserved ID for this widget if any, -1 otherwise.  (If no ID is
         *  reserved, widget will not be in the regular tabbing order */
        int m_irrlicht_widget_id;

        /** For animation purposes (see method 'move') */
        int target_x, target_y, target_w, target_h;
        float x_speed, y_speed, w_speed, h_speed;

        /** Object representing this player */
        /** Local info about the player. */
        StateManager::ActivePlayer* m_associated_player;
        int m_player_id;

        /** Network info about the user. */
        NetworkPlayerProfile* m_associated_user;

        /** Internal name of the spinner; useful to interpret spinner events,
         *  which contain the name of the activated object */
        std::string spinnerID;

#ifdef DEBUG
        long m_magic_number;
#endif

    public:

        LEAK_CHECK()

        /** Sub-widgets created by this widget */
        PlayerNameSpinner* m_player_ident_spinner;
        KartStatsWidget* m_kart_stats;
        ModelViewWidget* m_model_view;
        LabelWidget* m_kart_name;

        KartSelectionScreen* m_parent_screen;

        irr::gui::IGUIStaticText* m_ready_text;

        core::stringw deviceName;
        std::string m_kartInternalName;

        bool m_not_updated_yet;

        PlayerKartWidget(KartSelectionScreen* parent,
                         StateManager::ActivePlayer* associated_player,
                         NetworkPlayerProfile* associated_user,
                         core::recti area, const int player_id,
                         std::string kart_group,
                         const int irrlicht_idget_id=-1);
        // ------------------------------------------------------------------------

        ~PlayerKartWidget();

        // ------------------------------------------------------------------------
        /** Called when players are renumbered (changes the player ID) */
        void setPlayerID(const int newPlayerID);

        // ------------------------------------------------------------------------
        /** Returns the ID of this player */
        int getPlayerID() const;

        // ------------------------------------------------------------------------
        /** Add the widgets to the current screen */
        virtual void add();

        // ------------------------------------------------------------------------
        /** Get the associated ActivePlayer object*/
        StateManager::ActivePlayer* getAssociatedPlayer();

        // ------------------------------------------------------------------------
        /** Starts a 'move/resize' animation, by simply passing destination coords.
         *  The animation will then occur on each call to 'onUpdate'. */
        void move(const int x, const int y, const int w, const int h);

        // ------------------------------------------------------------------------
        /** Call when player confirmed his identity and kart */
        void markAsReady();

        // ------------------------------------------------------------------------
        /** \return Whether this player confirmed his kart and indent selection */
        bool isReady();

        // ------------------------------------------------------------------------
        /** \return Whether this player is handicapped or not */
        bool isHandicapped();

        // -------------------------------------------------------------------------
        /** Updates the animation (moving/shrinking/etc.) */
        void onUpdate(float delta);

        // -------------------------------------------------------------------------
        /** Event callback */
        virtual GUIEngine::EventPropagation transmitEvent(
            GUIEngine::Widget* w,
            const std::string& originator,
            const int m_player_id);

        // -------------------------------------------------------------------------
        /** Sets the size of the widget as a whole, and placed children widgets
         * inside itself */
        void setSize(const int x, const int y, const int w, const int h);

        // -------------------------------------------------------------------------

        /** Sets which kart was selected for this player */
        void setKartInternalName(const std::string& whichKart);

        // -------------------------------------------------------------------------

        const std::string& getKartInternalName() const;

        // -------------------------------------------------------------------------

        /** \brief Event callback from ISpinnerConfirmListener */
        virtual GUIEngine::EventPropagation onSpinnerConfirmed();
    };   // PlayerKartWidget
}

#endif

