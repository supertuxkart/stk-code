//  SuperTuxKart - a fun racing game with go-kart
//
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

#ifndef __TOP_LEVEL_CONT_HPP__
#define __TOP_LEVEL_CONT_HPP__


#include "guiengine/widget.hpp"
#include "utils/log.hpp"
#include "utils/ptr_vector.hpp"

#include <cstring> // for NULL
#include <typeinfo> // for typeid


namespace GUIEngine
{
    class Widget;

    /**
     * \brief Represents a GUI widgets container.
     *
     * Abstract base class for both Screen and ModalDialog.
     *
     * \ingroup guiengine
     */
    class AbstractTopLevelContainer
    {
    protected:
        /** the widgets in this screen */
        PtrVector<Widget, HOLD>  m_widgets;

        /**
         *  AbstractTopLevelContainer is generally able to determine its first
         *  widget just fine, but in highly complex screens (e.g. multiplayer
         *  kart selection) you can help it by providing the first widget
         *  manually.
         */
        Widget*                  m_first_widget;

        /**
         *  AbstractTopLevelContainer is generally able to determine its last
         *  widget just fine, but in highly complex screens (e.g. multiplayer
         *  kart selection) you can help it by providing the first widget
         *  manually.
         */
        Widget*                  m_last_widget;

                void         addWidgetsRecursively(PtrVector<Widget>& widgets,
                                    Widget* parent=NULL);

    public:
                             AbstractTopLevelContainer();
        virtual              ~AbstractTopLevelContainer() {}

        virtual int          getWidth() = 0;
        virtual int          getHeight() = 0;

        /** \return an object by name, or NULL if not found */
                Widget*      getWidget(const char* name);

        /** \return an object by irrlicht ID, or NULL if not found */
                Widget*      getWidget(const int id);

        /** This function searches and returns a widget by name, cast as specified type,
         *  if that widget is found and the type is correct.
         *  \param name The name of the widget to find
         *  \return an object by name, casted to specified type, or NULL if
         *  not found/wrong type */
        template <typename T>
                T*           getWidget(const char* name)
        {
            Widget* out = getWidget(name);
            T* outCasted = dynamic_cast<T*>( out );
            if (out != NULL && outCasted == NULL)
                Log::fatal("Screen::getWidget", "Widget '%s' of type '%s'"
                           "cannot be casted to requested type '%s'!\n", name,
                           typeid(*out).name(), typeid(T).name());
            return outCasted;
        }

        static  Widget*      getWidget(const char* name,
                                       PtrVector<Widget>* within_vector);
        static  Widget*      getWidget(const int id,
                                       PtrVector<Widget>* within_vector);

                Widget*      getFirstWidget(PtrVector<Widget>* within_vector=NULL);
                Widget*      getLastWidget(PtrVector<Widget>* within_vector=NULL);

                void         elementsWereDeleted(PtrVector<Widget>* within_vector = NULL);

                bool         isMyChild(Widget* widget) const;
    };   // AbstractTopLevelContainer

}   // namespace GUIEngine

#endif
