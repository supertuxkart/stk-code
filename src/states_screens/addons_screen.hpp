//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015 Lucas Baudin, Joerg Henrichs
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

#ifndef HEADER_ADDONS_SCREEN_HPP
#define HEADER_ADDONS_SCREEN_HPP\

#include "addons/addons_manager.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"

#include <list>

/* used for the installed/unsinstalled icons*/
namespace irr { namespace gui { class STKModifiedSpriteBank; } }

namespace GUIEngine { class LabelWidget; class Widget; }

class AddonsLoading;
struct DateFilter {
    core::stringw label;
    int year;
    int month;
    int day;
};

/**
  * \brief Addons screen
  * \ingroup states_screens
  */
class AddonsScreen : public GUIEngine::Screen,
                     public GUIEngine::ScreenSingleton<AddonsScreen>,
                     public GUIEngine::IListWidgetHeaderListener,
                     public GUIEngine::ITextBoxWidgetListener
{
    friend class GUIEngine::ScreenSingleton<AddonsScreen>;
private:
    AddonsScreen();
    AddonsLoading   *m_load;
    void             loadInformations();
    /** Icon for installed addon, which can be updated. */
    int              m_icon_needs_update;
    /** Icon for installed addons, no update available. */
    int              m_icon_installed;
    /** Icon for is not installed yet. */
    int              m_icon_not_installed;
    /** Icon for 'loading' */
    int              m_icon_loading;

    std::list<std::string> m_addon_queue;
    AddonsLoading * m_addons_loading;

    irr::gui::STKModifiedSpriteBank
                    *m_icon_bank;

    /** Currently selected type. */
    std::string      m_type;

    /** The currently selected index, used to re-select this item after
     *  addons_loading is being displayed. */
    int              m_selected_index;

    bool             m_reloading;

    bool             m_sort_desc;

    bool             m_cancelled;

    /** List of date filters **/
    std::vector<DateFilter> m_date_filters;

public:

    /** Load the addons into the main list.*/
    void loadList();

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void loadedFromFile() OVERRIDE;

    virtual void unloaded() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void eventCallback(GUIEngine::Widget* widget, const std::string& name,
                               const int playerID) OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void beforeAddingWidget() OVERRIDE;

    virtual void onColumnClicked(int column_id, bool sort_desc, bool sort_default) OVERRIDE;

    virtual void init() OVERRIDE;
    virtual void tearDown() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void onUpdate(float dt) OVERRIDE;

    /** \brief rebuild the list based on search text */
    virtual void onTextUpdated() OVERRIDE
    {
        loadList();
    }

    void    setLastSelected();

};

#endif
