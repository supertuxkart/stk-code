// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CGUIStaticText.h"
#ifdef _IRR_COMPILE_WITH_GUI_

#include "IGUISkin.h"
#include "IGUIEnvironment.h"
#include "IGUIFont.h"
#include "IVideoDriver.h"
#include "rect.h"

namespace irr
{
namespace gui
{

//! constructor
CGUIStaticText::CGUIStaticText(const core::stringw& text, bool border,
			IGUIEnvironment* environment, IGUIElement* parent,
			s32 id, const core::rect<s32>& rectangle,
			bool background)
: IGUIStaticText(environment, parent, id, rectangle),
	HAlign(EGUIA_UPPERLEFT), VAlign(EGUIA_UPPERLEFT),
	Border(border), OverrideColorEnabled(false), OverrideBGColorEnabled(false), WordWrap(false), Background(background),
	RestrainTextInside(true),
	OverrideColor(video::SColor(101,255,255,255)), BGColor(video::SColor(101,210,210,210)),
	OverrideFont(0), LastBreakFont(0), m_use_glyph_layouts_only(false)
{
	#ifdef _DEBUG
	setDebugName("CGUIStaticText");
	#endif

	Text = text;
	if (environment && environment->getSkin())
	{
		BGColor = environment->getSkin()->getColor(gui::EGDC_3D_FACE);
	}
}


//! destructor
CGUIStaticText::~CGUIStaticText()
{
	if (OverrideFont)
		OverrideFont->drop();
}


//! draws the element and its children
void CGUIStaticText::draw()
{
	if (!IsVisible)
		return;

	IGUISkin* skin = Environment->getSkin();
	if (!skin)
		return;
	video::IVideoDriver* driver = Environment->getVideoDriver();

	core::rect<s32> frameRect(AbsoluteRect);

	// draw background

	if (Background)
	{
		if ( !OverrideBGColorEnabled )	// skin-colors can change
			BGColor = skin->getColor(gui::EGDC_3D_FACE);

		driver->draw2DRectangle(BGColor, frameRect, &AbsoluteClippingRect);
	}

	// draw the border

	if (Border)
	{
		skin->draw3DSunkenPane(this, 0, true, false, frameRect, &AbsoluteClippingRect);
		frameRect.UpperLeftCorner.X += skin->getSize(EGDS_TEXT_DISTANCE_X);
	}

	// draw the text layout
	if (!m_glyph_layouts.empty())
	{
		IGUIFont* font = getActiveFont();

		if (font)
		{
			if (font != LastBreakFont)
				breakText();

			core::rect<s32> r = frameRect;
			auto dim = getGlyphLayoutsDimension(m_glyph_layouts,
				font->getHeightPerLine(), font->getInverseShaping(), font->getScale());

			s32 totalHeight = dim.Height;
			if (VAlign == EGUIA_CENTER)
			{
				r.UpperLeftCorner.Y = r.getCenter().Y - (totalHeight / 2);
			}
			else if (VAlign == EGUIA_LOWERRIGHT)
			{
				r.UpperLeftCorner.Y = r.LowerRightCorner.Y - totalHeight;
			}


			if (HAlign == EGUIA_LOWERRIGHT)
			{
				r.UpperLeftCorner.X = frameRect.LowerRightCorner.X - dim.Width;
			}

			font->draw(m_glyph_layouts, r,
				OverrideColorEnabled ? OverrideColor : skin->getColor(isEnabled() ? EGDC_BUTTON_TEXT : EGDC_GRAY_TEXT),
				HAlign == EGUIA_CENTER, false, (RestrainTextInside ? &AbsoluteClippingRect : NULL));
		}
	}

	IGUIElement::draw();
}


//! Sets another skin independent font.
void CGUIStaticText::setOverrideFont(IGUIFont* font)
{
	if (OverrideFont == font)
		return;

	if (OverrideFont)
		OverrideFont->drop();

	OverrideFont = font;

	if (OverrideFont)
		OverrideFont->grab();

	breakText();
}

//! Gets the override font (if any)
IGUIFont * CGUIStaticText::getOverrideFont() const
{
	return OverrideFont;
}

//! Get the font which is used right now for drawing
IGUIFont* CGUIStaticText::getActiveFont() const
{
	if ( OverrideFont )
		return OverrideFont;
	IGUISkin* skin = Environment->getSkin();
	if (skin)
		return skin->getFont();
	return 0;
}

//! Sets another color for the text.
void CGUIStaticText::setOverrideColor(video::SColor color)
{
	OverrideColor = color;
	OverrideColorEnabled = true;
}


//! Sets another color for the text.
void CGUIStaticText::setBackgroundColor(video::SColor color)
{
	BGColor = color;
	OverrideBGColorEnabled = true;
	Background = true;
}


//! Sets whether to draw the background
void CGUIStaticText::setDrawBackground(bool draw)
{
	Background = draw;
}


//! Gets the background color
video::SColor CGUIStaticText::getBackgroundColor() const
{
	return BGColor;
}


//! Checks if background drawing is enabled
bool CGUIStaticText::isDrawBackgroundEnabled() const
{
	_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
	return Background;
}


//! Sets whether to draw the border
void CGUIStaticText::setDrawBorder(bool draw)
{
	Border = draw;
}


//! Checks if border drawing is enabled
bool CGUIStaticText::isDrawBorderEnabled() const
{
	_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
	return Border;
}


void CGUIStaticText::setTextRestrainedInside(bool restrainTextInside)
{
	RestrainTextInside = restrainTextInside;
}


bool CGUIStaticText::isTextRestrainedInside() const
{
	return RestrainTextInside;
}


void CGUIStaticText::setTextAlignment(EGUI_ALIGNMENT horizontal, EGUI_ALIGNMENT vertical)
{
	HAlign = horizontal;
	VAlign = vertical;
}


video::SColor CGUIStaticText::getOverrideColor() const
{
	return OverrideColor;
}


//! Sets if the static text should use the overide color or the
//! color in the gui skin.
void CGUIStaticText::enableOverrideColor(bool enable)
{
	OverrideColorEnabled = enable;
}


bool CGUIStaticText::isOverrideColorEnabled() const
{
	_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
	return OverrideColorEnabled;
}


//! Enables or disables word wrap for using the static text as
//! multiline text control.
void CGUIStaticText::setWordWrap(bool enable)
{
	WordWrap = enable;
	breakText();
}


bool CGUIStaticText::isWordWrapEnabled() const
{
	_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
	return WordWrap;
}


//! Breaks the single text line.
void CGUIStaticText::breakText()
{
	m_glyph_layouts.clear();

	IGUISkin* skin = Environment->getSkin();
	IGUIFont* font = getActiveFont();
	if (!font)
		return;

	LastBreakFont = font;

	f32 elWidth = (f32)RelativeRect.getWidth();
	if (Border)
		elWidth -= 2*skin->getSize(EGDS_TEXT_DISTANCE_X);
	if (!m_use_glyph_layouts_only)
		font->initGlyphLayouts(Text, m_glyph_layouts);
	if (WordWrap)
		breakGlyphLayouts(m_glyph_layouts, elWidth, font->getInverseShaping(), font->getScale());
}


//! Sets the new caption of this element.
void CGUIStaticText::setText(const core::stringw& text)
{
	if (text == Text)
		return;
	IGUIElement::setText(text);
	breakText();
}


void CGUIStaticText::updateAbsolutePosition()
{
	IGUIElement::updateAbsolutePosition();
	breakText();
}


//! Returns the height of the text in pixels when it is drawn.
s32 CGUIStaticText::getTextHeight() const
{
	IGUIFont* font = getActiveFont();
	if (!font)
		return 0;

	auto dim = getGlyphLayoutsDimension(m_glyph_layouts,
		font->getHeightPerLine(), font->getInverseShaping(), font->getScale());

	return dim.Height;
}


s32 CGUIStaticText::getTextWidth() const
{
	IGUIFont * font = getActiveFont();
	if (!font)
		return 0;

	auto dim = getGlyphLayoutsDimension(m_glyph_layouts,
		font->getHeightPerLine(), font->getInverseShaping(), font->getScale());

	return dim.Width;
}


//! Writes attributes of the element.
//! Implement this to expose the attributes of your element for
//! scripting languages, editors, debuggers or xml serialization purposes.
void CGUIStaticText::serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options=0) const
{
	IGUIStaticText::serializeAttributes(out,options);

	out->addBool	("Border",              Border);
	out->addBool	("OverrideColorEnabled",OverrideColorEnabled);
	out->addBool	("OverrideBGColorEnabled",OverrideBGColorEnabled);
	out->addBool	("WordWrap",		WordWrap);
	out->addBool	("Background",          Background);
	out->addBool	("RestrainTextInside",  RestrainTextInside);
	out->addColor	("OverrideColor",       OverrideColor);
	out->addColor	("BGColor",       	BGColor);
	out->addEnum	("HTextAlign",          HAlign, GUIAlignmentNames);
	out->addEnum	("VTextAlign",          VAlign, GUIAlignmentNames);

	// out->addFont ("OverrideFont",	OverrideFont);
}


//! Reads attributes of the element
void CGUIStaticText::deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options=0)
{
	IGUIStaticText::deserializeAttributes(in,options);

	Border = in->getAttributeAsBool("Border");
	enableOverrideColor(in->getAttributeAsBool("OverrideColorEnabled"));
	OverrideBGColorEnabled = in->getAttributeAsBool("OverrideBGColorEnabled");
	setWordWrap(in->getAttributeAsBool("WordWrap"));
	Background = in->getAttributeAsBool("Background");
	RestrainTextInside = in->getAttributeAsBool("RestrainTextInside");
	OverrideColor = in->getAttributeAsColor("OverrideColor");
	BGColor = in->getAttributeAsColor("BGColor");

	setTextAlignment( (EGUI_ALIGNMENT) in->getAttributeAsEnumeration("HTextAlign", GUIAlignmentNames),
                      (EGUI_ALIGNMENT) in->getAttributeAsEnumeration("VTextAlign", GUIAlignmentNames));

	// OverrideFont = in->getAttributeAsFont("OverrideFont");
}


} // end namespace gui
} // end namespace irr

#endif // _IRR_COMPILE_WITH_GUI_

