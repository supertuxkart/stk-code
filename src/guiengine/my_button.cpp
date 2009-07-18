// The contents of CGUIButtom.cpp from irrLicht sources, but with a patch applied.
// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include <irrlicht.h>
#include "guiengine/my_button.hpp"

using namespace GUIEngine;

//! constructor
MyGUIButton::MyGUIButton(IGUIEnvironment* environment, IGUIElement* parent,
                       s32 id, core::rect<s32> rectangle, bool noclip)
: IGUIButton(environment, parent, id, rectangle), Pressed(false),
IsPushButton(false), UseAlphaChannel(false), Border(true),
ClickTime(0), SpriteBank(0), OverrideFont(0), Image(0), PressedImage(0)
{
#ifdef _DEBUG
	setDebugName("MyGUIButton");
#endif
	setNotClipped(noclip);
    
	// Initialize the sprites.
	for (u32 i=0; i<EGBS_COUNT; ++i)
		ButtonSprites[i].Index = -1;
    
	// This element can be tabbed.
	setTabStop(true);
	setTabOrder(-1);
}


//! destructor
MyGUIButton::~MyGUIButton()
{
	if (OverrideFont)
		OverrideFont->drop();
    
	if (Image)
		Image->drop();
    
	if (PressedImage)
		PressedImage->drop();
    
	if (SpriteBank)
		SpriteBank->drop();
}


//! Sets if the button should use the skin to draw its border
void MyGUIButton::setDrawBorder(bool border)
{
	Border = border;
}


void MyGUIButton::setSpriteBank(IGUISpriteBank* sprites)
{
	if (sprites)
		sprites->grab();
    
	if (SpriteBank)
		SpriteBank->drop();
    
	SpriteBank = sprites;
}


void MyGUIButton::setSprite(EGUI_BUTTON_STATE state, s32 index, video::SColor color, bool loop)
{
	if (SpriteBank)
	{
		ButtonSprites[(u32)state].Index	= index;
		ButtonSprites[(u32)state].Color	= color;
		ButtonSprites[(u32)state].Loop	= loop;
	}
	else
	{
		ButtonSprites[(u32)state].Index = -1;
	}
}


//! called if an event happened.
bool MyGUIButton::OnEvent(const SEvent& event)
{
	if (!IsEnabled)
		return IGUIElement::OnEvent(event);
    
	switch(event.EventType)
	{
        case EET_KEY_INPUT_EVENT:
            if (event.KeyInput.PressedDown &&
                (event.KeyInput.Key == KEY_RETURN || 
                 event.KeyInput.Key == KEY_SPACE))
            {
                if (!IsPushButton)
                    setPressed(true);
                else
                    setPressed(!Pressed);
                
                return true;
            }
            if (Pressed && !IsPushButton && event.KeyInput.PressedDown && event.KeyInput.Key == KEY_ESCAPE)
            {
                setPressed(false);
                return true;
            }
            else
                if (!event.KeyInput.PressedDown && Pressed &&
                    (event.KeyInput.Key == KEY_RETURN || 
                     event.KeyInput.Key == KEY_SPACE))
                {
                    //Environment->removeFocus(this);
                    
                    if (!IsPushButton)
                        setPressed(false);
                    
                    if (Parent)
                    {
                        SEvent newEvent;
                        newEvent.EventType = EET_GUI_EVENT;
                        newEvent.GUIEvent.Caller = this;
                        newEvent.GUIEvent.Element = 0;
                        newEvent.GUIEvent.EventType = EGET_BUTTON_CLICKED;
                        Parent->OnEvent(newEvent);
                    }
                    return true;
                }
            break;
        case EET_GUI_EVENT:
            if (event.GUIEvent.EventType == EGET_ELEMENT_FOCUS_LOST)
            {
                if (event.GUIEvent.Caller == this && !IsPushButton)
                    setPressed(false);
            }
            break;
        case EET_MOUSE_INPUT_EVENT:
            if (event.MouseInput.Event == EMIE_LMOUSE_PRESSED_DOWN)
            {
                if (Environment->hasFocus(this) &&
                    !AbsoluteClippingRect.isPointInside(core::position2d<s32>(event.MouseInput.X, event.MouseInput.Y)))
                {
					Environment->removeFocus(this);
					return false;
                }
                
                if (!IsPushButton)
                    setPressed(true);
                
                Environment->setFocus(this);
                return true;
            }
            else
                if (event.MouseInput.Event == EMIE_LMOUSE_LEFT_UP)
                {
                    bool wasPressed = Pressed;
                    //Environment->removeFocus(this);
                    
                    if ( !AbsoluteClippingRect.isPointInside( core::position2d<s32>(event.MouseInput.X, event.MouseInput.Y ) ) )
                    {
                        if (!IsPushButton)
                            setPressed(false);
                        return true;
                    }
                    
                    if (!IsPushButton)
                        setPressed(false);
                    else
                    {
                        setPressed(!Pressed);
                    }
                    
                    if ((!IsPushButton && wasPressed && Parent) ||
                        (IsPushButton && wasPressed != Pressed))
                    {
                        SEvent newEvent;
                        newEvent.EventType = EET_GUI_EVENT;
                        newEvent.GUIEvent.Caller = this;
                        newEvent.GUIEvent.Element = 0;
                        newEvent.GUIEvent.EventType = EGET_BUTTON_CLICKED;
                        Parent->OnEvent(newEvent);
                    }
                    
                    return true;
                }
            break;
        default:
            break;
	}
    
	return Parent ? Parent->OnEvent(event) : false;
}

#define AutoScale true

//! draws the element and its children
void MyGUIButton::draw()
{
	if (!IsVisible)
		return;
    
	IGUISkin* skin = Environment->getSkin();
	video::IVideoDriver* driver = Environment->getVideoDriver();
    
	IGUIFont* font = OverrideFont;
	if (!OverrideFont)
		font = skin->getFont(EGDF_BUTTON);
    
	core::rect<s32> rect = AbsoluteRect;
    
	// todo:	move sprite up and text down if the pressed state has a sprite
	//			draw sprites for focused and mouse-over 
	core::position2di spritePos = AbsoluteRect.getCenter();
    
	if (!Pressed)
	{
		if (Border)
			skin->draw3DButtonPaneStandard(this, rect, &AbsoluteClippingRect);
        
		if (Image)
		{
            if(!AutoScale)
            {
                core::position2d<s32> pos = AbsoluteRect.getCenter();
                pos.X -= ImageRect.getWidth() / 2;
                pos.Y -= ImageRect.getHeight() / 2;
                
                driver->draw2DImage(Image, pos, ImageRect, &AbsoluteClippingRect,
                                    video::SColor(255,255,255,255), UseAlphaChannel);
            }
            else
            {
                driver->draw2DImage(Image, AbsoluteRect, ImageRect,
                                    &AbsoluteClippingRect, 0, UseAlphaChannel);
            }
            /*
            core::position2d<s32> pos = AbsoluteRect.getCenter();
			pos.X -= ImageRect.getWidth() / 2;
			pos.Y -= ImageRect.getHeight() / 2;
            
			driver->draw2DImage(Image, pos, ImageRect, &AbsoluteClippingRect, 
                                video::SColor(255,255,255,255), UseAlphaChannel);
             */
		}
		if (SpriteBank && ButtonSprites[EGBS_BUTTON_UP].Index != -1)
		{
			// draw pressed sprite
            /*
			SpriteBank->draw2DSprite(ButtonSprites[EGBS_BUTTON_UP].Index, spritePos, 
                                     &AbsoluteClippingRect, ButtonSprites[EGBS_BUTTON_UP].Color, ClickTime, os::Timer::getTime(), 
                                     ButtonSprites[EGBS_BUTTON_UP].Loop, true);
             */
            printf("Sorry, sprite support for buttons was disabled in order to avoid os-specific timing stuff\n");
		}
	}
	else
	{
		if (Border)
			skin->draw3DButtonPanePressed(this, rect, &AbsoluteClippingRect);
        
		if (PressedImage)
		{

            
            if(!AutoScale)
            {
                core::position2d<s32> pos = AbsoluteRect.getCenter();
                pos.X -= PressedImageRect.getWidth() / 2;
                pos.Y -= PressedImageRect.getHeight() / 2;
                // patch by Alan Tyndall/Jonas Petersen
                if (Image == PressedImage && PressedImageRect == ImageRect)
                {
                    pos.X += 1;
                    pos.Y += 1;
                }
                
                driver->draw2DImage(Image, pos, ImageRect, &AbsoluteClippingRect,
                                    video::SColor(255,255,255,255), UseAlphaChannel);
            }
            else
            {
                driver->draw2DImage(Image, AbsoluteRect, ImageRect,
                                    &AbsoluteClippingRect, 0, UseAlphaChannel);
            }
            
		}
        
		if (SpriteBank && ButtonSprites[EGBS_BUTTON_DOWN].Index != -1)
		{
            /*
			// draw sprite
			SpriteBank->draw2DSprite(ButtonSprites[EGBS_BUTTON_DOWN].Index, spritePos, 
                                     &AbsoluteClippingRect, ButtonSprites[EGBS_BUTTON_DOWN].Color, ClickTime, os::Timer::getTime(), 
                                     ButtonSprites[EGBS_BUTTON_DOWN].Loop, true);
             */
            printf("Sorry, sprite support for buttons was disabled in order to avoid os-specific timing stuff\n");
		}
        
	}
    
	if (Text.size())
	{
		rect = AbsoluteRect;
        if (Pressed)
			rect.UpperLeftCorner.Y += 2;
        
		if (font)
			font->draw(Text.c_str(), rect,
                       skin->getColor(IsEnabled ? EGDC_BUTTON_TEXT : EGDC_GRAY_TEXT), true, true, 
                       &AbsoluteClippingRect);
	}
    
	IGUIElement::draw();
}


//! sets another skin independent font. if this is set to zero, the button uses the font of the skin.
void MyGUIButton::setOverrideFont(IGUIFont* font)
{
	if (OverrideFont)
		OverrideFont->drop();
    
	OverrideFont = font;
    
	if (OverrideFont)
		OverrideFont->grab();
}


//! Sets an image which should be displayed on the button when it is in normal state. 
void MyGUIButton::setImage(video::ITexture* image)
{
	if (Image)
		Image->drop();
    
	Image = image;
	if (image)
		ImageRect = core::rect<s32>(core::position2d<s32>(0,0), image->getOriginalSize());
    
	if (Image)
		Image->grab();
    
	if (!PressedImage)
		setPressedImage(Image);
}


//! Sets the image which should be displayed on the button when it is in its normal state.
void MyGUIButton::setImage(video::ITexture* image, const core::rect<s32>& pos)
{
	if (Image)
		Image->drop();
    
	Image = image;
	ImageRect = pos;
    
	if (Image)
		Image->grab();
    
	if (!PressedImage)
		setPressedImage(Image, pos);
}


//! Sets an image which should be displayed on the button when it is in pressed state. 
void MyGUIButton::setPressedImage(video::ITexture* image)
{
	if (PressedImage)
		PressedImage->drop();
    
	PressedImage = image;
	if (image)
		PressedImageRect = core::rect<s32>(core::position2d<s32>(0,0), image->getOriginalSize());
    
	if (PressedImage)
		PressedImage->grab();
}


//! Sets the image which should be displayed on the button when it is in its pressed state.
void MyGUIButton::setPressedImage(video::ITexture* image, const core::rect<s32>& pos)
{
	if (PressedImage)
		PressedImage->drop();
    
	PressedImage = image;
	PressedImageRect = pos;
    
	if (PressedImage)
		PressedImage->grab();
}


//! Sets if the button should behave like a push button. Which means it
//! can be in two states: Normal or Pressed. With a click on the button,
//! the user can change the state of the button.
void MyGUIButton::setIsPushButton(bool isPushButton)
{
	IsPushButton = isPushButton;
}


//! Returns if the button is currently pressed
bool MyGUIButton::isPressed() const
{
	_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
	return Pressed;
}


//! Sets the pressed state of the button if this is a pushbutton
void MyGUIButton::setPressed(bool pressed)
{
	if (Pressed != pressed)
	{
		//ClickTime = os::Timer::getTime();
		Pressed = pressed;
	}
}


//! Returns whether the button is a push button
bool MyGUIButton::isPushButton() const
{
	_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
	return IsPushButton;
}


//! Sets if the alpha channel should be used for drawing images on the button (default is false)
void MyGUIButton::setUseAlphaChannel(bool useAlphaChannel)
{
	UseAlphaChannel = useAlphaChannel;
}


//! Returns if the alpha channel should be used for drawing images on the button
bool MyGUIButton::isAlphaChannelUsed() const
{
	_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
	return UseAlphaChannel;
}


bool MyGUIButton::isDrawingBorder() const
{
	_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
	return Border;
}


//! Writes attributes of the element.
void MyGUIButton::serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options=0) const
{
	IGUIButton::serializeAttributes(out,options);
    
	out->addBool	("PushButton",		IsPushButton );
	if (IsPushButton)
		out->addBool("Pressed",		Pressed);
    
	out->addTexture ("Image",		Image);
	out->addRect	("ImageRect",		ImageRect);
	out->addTexture	("PressedImage",	PressedImage);
	out->addRect	("PressedImageRect",	PressedImageRect);
    
	out->addBool	("Border",		Border);
	out->addBool	("UseAlphaChannel",	UseAlphaChannel);
    
	//   out->addString  ("OverrideFont",	OverrideFont);
}


//! Reads attributes of the element
void MyGUIButton::deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options=0)
{
	IGUIButton::deserializeAttributes(in,options);
    
	IsPushButton	= in->getAttributeAsBool("PushButton");
	Pressed		= IsPushButton ? in->getAttributeAsBool("Pressed") : false;
    
	core::rect<s32> rec = in->getAttributeAsRect("ImageRect");
	if (rec.isValid())
		setImage( in->getAttributeAsTexture("Image"), rec);
	else
		setImage( in->getAttributeAsTexture("Image") );
    
	rec = in->getAttributeAsRect("PressedImageRect");
	if (rec.isValid())
		setPressedImage( in->getAttributeAsTexture("PressedImage"), rec);
	else
		setPressedImage( in->getAttributeAsTexture("PressedImage") );
    
	setDrawBorder(in->getAttributeAsBool("Border"));
	UseAlphaChannel = in->getAttributeAsBool("UseAlphaChannel");
    
	//   setOverrideFont(in->getAttributeAsString("OverrideFont"));
    
	updateAbsolutePosition();
}
