#ifndef HEADER_SKIN_HPP
#define HEADER_SKIN_HPP

#include <irrlicht.h>
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

namespace GUIEngine
{

class Skin : public IGUISkin
{
    IGUISkin* m_fallback_skin;
public:
    Skin(IGUISkin* fallback_skin);
    ~Skin();
    
    virtual void 	draw2DRectangle (IGUIElement *element, const video::SColor &color, const core::rect< s32 > &pos, const core::rect< s32 > *clip);

    virtual void 	draw3DButtonPanePressed (IGUIElement *element, const core::rect< s32 > &rect, const core::rect< s32 > *clip);

    virtual void 	draw3DButtonPaneStandard (IGUIElement *element, const core::rect< s32 > &rect, const core::rect< s32 > *clip);

    virtual void 	draw3DMenuPane (IGUIElement *element, const core::rect< s32 > &rect, const core::rect< s32 > *clip);

    virtual void 	draw3DSunkenPane (IGUIElement *element, video::SColor bgcolor, bool flat, bool fillBackGround, const core::rect< s32 > &rect, const core::rect< s32 > *clip);

    virtual void 	draw3DTabBody (IGUIElement *element, bool border, bool background, const core::rect< s32 > &rect, const core::rect< s32 > *clip, s32 tabHeight=-1, gui::EGUI_ALIGNMENT alignment=EGUIA_UPPERLEFT);

    virtual void 	draw3DTabButton (IGUIElement *element, bool active, const core::rect< s32 > &rect, const core::rect< s32 > *clip, gui::EGUI_ALIGNMENT alignment=EGUIA_UPPERLEFT);

    virtual void 	draw3DToolBar (IGUIElement *element, const core::rect< s32 > &rect, const core::rect< s32 > *clip);

    virtual core::rect< s32 > 	draw3DWindowBackground (IGUIElement *element, bool drawTitleBar, video::SColor titleBarColor, const core::rect< s32 > &rect, const core::rect< s32 > *clip);

    virtual void 	drawIcon (IGUIElement *element, EGUI_DEFAULT_ICON icon, const core::position2di position, u32 starttime, u32 currenttime, bool loop=false, const core::rect< s32 > *clip=NULL);

    virtual video::SColor 	getColor (EGUI_DEFAULT_COLOR color) const;

    virtual const wchar_t * 	getDefaultText (EGUI_DEFAULT_TEXT text) const;

    virtual IGUIFont * 	getFont (EGUI_DEFAULT_FONT which=EGDF_DEFAULT) const ;

    virtual u32 	getIcon (EGUI_DEFAULT_ICON icon) const ;

    virtual s32 	getSize (EGUI_DEFAULT_SIZE size) const ;

    virtual IGUISpriteBank * 	getSpriteBank () const ;

    //virtual EGUI_SKIN_TYPE 	getType () const;

    virtual void 	setColor (EGUI_DEFAULT_COLOR which, video::SColor newColor);

    virtual void 	setDefaultText (EGUI_DEFAULT_TEXT which, const wchar_t *newText);

    virtual void 	setFont (IGUIFont *font, EGUI_DEFAULT_FONT which=EGDF_DEFAULT);

    virtual void 	setIcon (EGUI_DEFAULT_ICON icon, u32 index);

    virtual void 	setSize (EGUI_DEFAULT_SIZE which, s32 size);

    virtual void 	setSpriteBank (IGUISpriteBank *bank);

};
}
#endif