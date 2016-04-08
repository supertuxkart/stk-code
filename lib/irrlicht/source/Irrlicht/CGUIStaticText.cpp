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
#include "utfwrapping.h"

namespace irr
{
namespace gui
{

//! constructor
CGUIStaticText::CGUIStaticText(const wchar_t* text, bool border,
			IGUIEnvironment* environment, IGUIElement* parent,
			s32 id, const core::rect<s32>& rectangle,
			bool background)
: IGUIStaticText(environment, parent, id, rectangle),
	HAlign(EGUIA_UPPERLEFT), VAlign(EGUIA_UPPERLEFT),
	Border(border), OverrideColorEnabled(false), OverrideBGColorEnabled(false), WordWrap(false), Background(background),
	RestrainTextInside(true), RightToLeft(false),
	OverrideColor(video::SColor(101,255,255,255)), BGColor(video::SColor(101,210,210,210)),
	OverrideFont(0), LastBreakFont(0)
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

	// draw the text
	if (Text.size())
	{
		IGUIFont* font = getActiveFont();

		if (font)
		{
			if (!WordWrap)
			{
				if (VAlign == EGUIA_LOWERRIGHT)
				{
					frameRect.UpperLeftCorner.Y = frameRect.LowerRightCorner.Y -
						font->getDimension(L"A").Height - font->getKerningHeight();
				}
				if (HAlign == EGUIA_LOWERRIGHT)
				{
					frameRect.UpperLeftCorner.X = frameRect.LowerRightCorner.X -
						font->getDimension(Text.c_str()).Width;
				}

				font->draw(Text.c_str(), frameRect,
					OverrideColorEnabled ? OverrideColor : skin->getColor(isEnabled() ? EGDC_BUTTON_TEXT : EGDC_GRAY_TEXT),
					HAlign == EGUIA_CENTER, VAlign == EGUIA_CENTER, (RestrainTextInside ? &AbsoluteClippingRect : NULL));
			}
			else
			{
				if (font != LastBreakFont)
					breakText();

				core::rect<s32> r = frameRect;
				s32 height = font->getDimension(L"A").Height + font->getKerningHeight();
				s32 totalHeight = height * BrokenText.size();
				if (VAlign == EGUIA_CENTER)
				{
					r.UpperLeftCorner.Y = r.getCenter().Y - (totalHeight / 2);
				}
				else if (VAlign == EGUIA_LOWERRIGHT)
				{
					r.UpperLeftCorner.Y = r.LowerRightCorner.Y - totalHeight;
				}

				for (u32 i=0; i<BrokenText.size(); ++i)
				{
					if (HAlign == EGUIA_LOWERRIGHT)
					{
						r.UpperLeftCorner.X = frameRect.LowerRightCorner.X -
							font->getDimension(BrokenText[i].c_str()).Width;
					}

					font->draw(BrokenText[i].c_str(), r,
						OverrideColorEnabled ? OverrideColor : skin->getColor(isEnabled() ? EGDC_BUTTON_TEXT : EGDC_GRAY_TEXT),
						HAlign == EGUIA_CENTER, false, (RestrainTextInside ? &AbsoluteClippingRect : NULL));

					r.LowerRightCorner.Y += height;
					r.UpperLeftCorner.Y += height;
				}
			}
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


void CGUIStaticText::setRightToLeft(bool rtl)
{
	if (RightToLeft != rtl)
	{
		RightToLeft = rtl;
		breakText();
	}
}


bool CGUIStaticText::isRightToLeft() const
{
	return RightToLeft;
}


//! Breaks the single text line.
void CGUIStaticText::breakText()
{
	if (!WordWrap)
		return;

	BrokenText.clear();

	IGUISkin* skin = Environment->getSkin();
	IGUIFont* font = getActiveFont();
	if (!font)
		return;

	LastBreakFont = font;

	core::stringw line;
	core::stringw word;
	core::stringw whitespace;
	s32 size = Text.size();
	s32 length = 0;
	s32 elWidth = RelativeRect.getWidth();
	if (Border)
		elWidth -= 2*skin->getSize(EGDS_TEXT_DISTANCE_X);
	wchar_t c;

	// We have to deal with right-to-left and left-to-right differently
	// However, most parts of the following code is the same, it's just
	// some order and boundaries which change.
	if (!RightToLeft)
	{
		// regular (left-to-right)
		for (s32 i=0; i<size; ++i)
		{
			c = Text[i];
			bool lineBreak = false;

			if (c == L'\r') // Mac or Windows breaks
			{
				lineBreak = true;
				if (Text[i+1] == L'\n') // Windows breaks
				{
					Text.erase(i+1);
					--size;
				}
				c = '\0';
			}
			else if (c == L'\n') // Unix breaks
			{
				lineBreak = true;
				c = '\0';
			}
			word += c;

			if (word.size())
			{
				const s32 wordlgth = font->getDimension(word.c_str()).Width;

				if (length && (length + wordlgth > elWidth))
				{	// too long to fit inside
					// break to next line
					unsigned int where = 1;
					while (where != line.size()) //Find the first breakable position
					{
						if (UtfNoEnding(Text[i - where])   ||   //Prevent unsuitable character from displaying
							UtfNoStarting(Text[i - where])  ||   //at the position of starting or ending of a line
							UtfNoStarting(Text[i + 1 - where])) //Handle case which more than one non-newline-starting characters are together
						{
							where++;
							continue;
						}
						if (breakable(Text[i - where]))
							break;
						else
							where++;
					}
					if (where != line.size())
					{
						core::stringw first  = line.subString(0, line.size() + 1 - where);
						core::stringw second = line.subString(line.size() + 1 - where , where - 1);
						if (first.lastChar() == wchar_t(0x00AD))
							BrokenText.push_back(first + L"-"); //Print the Unicode Soft HYphen (SHY / 00AD) character
						else
							BrokenText.push_back(first);
						const s32 secondLength = font->getDimension(second.c_str()).Width;

						length = secondLength + wordlgth;
						line = second + word;
					}
					else if (breakable(c) || UtfNoEnding(c) || UtfNoStarting(c)) //Unusual case
					{
						BrokenText.push_back(line);	//Force breaking to next line too if last word is breakable,
						line = word;						//it happens when someone writes too many non-newline-starting
						length = wordlgth;				//chars in the first line, so we ignore the rules.
					}
					// No suitable place to break words, so there's nothing more we can do
					// break to next line
					else
					{
						line += word;
						length += wordlgth;
					}
				}
				else
				{
					line += word;
					length += wordlgth;
				}

				word = L"";

			}
			// compute line break
			if (lineBreak)
			{
				line += word;
				BrokenText.push_back(line);
				line = L"";
				word = L"";
				length = 0;
			}
		}

		line += word;
		BrokenText.push_back(line);
	}
	else
	{
		// right-to-left
		for (s32 i=size; i>=0; --i)
		{
			c = Text[i];
			bool lineBreak = false;

			if (c == L'\r') // Mac or Windows breaks
			{
				lineBreak = true;
				if ((i>0) && Text[i-1] == L'\n') // Windows breaks
				{
					Text.erase(i-1);
					--size;
				}
				c = '\0';
			}
			else if (c == L'\n') // Unix breaks
			{
				lineBreak = true;
				c = '\0';
			}

			if (c==L' ' || c==0 || i==0)
			{
				if (word.size())
				{
					// here comes the next whitespace, look if
					// we must break the last word to the next line.
					const s32 whitelgth = font->getDimension(whitespace.c_str()).Width;
					const s32 wordlgth = font->getDimension(word.c_str()).Width;

					if (length && (length + wordlgth + whitelgth > elWidth))
					{
						// break to next line
						BrokenText.push_back(line);
						length = wordlgth;
						line = word;
					}
					else
					{
						// add word to line
						line = whitespace + line;
						line = word + line;
						length += whitelgth + wordlgth;
					}

					word = L"";
					whitespace = L"";
				}

				if (c != 0)
					whitespace = core::stringw(&c, 1) + whitespace;

				// compute line break
				if (lineBreak)
				{
					line = whitespace + line;
					line = word + line;
					BrokenText.push_back(line);
					line = L"";
					word = L"";
					whitespace = L"";
					length = 0;
				}
			}
			else
			{
				// yippee this is a word..
				word = core::stringw(&c, 1) + word;
			}
		}

		line = whitespace + line;
		line = word + line;
		BrokenText.push_back(line);
	}
}


//! Sets the new caption of this element.
void CGUIStaticText::setText(const wchar_t* text)
{
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

	s32 height = font->getDimension(L"A").Height + font->getKerningHeight();

	if (WordWrap)
		height *= BrokenText.size();

	return height;
}


s32 CGUIStaticText::getTextWidth() const
{
	IGUIFont * font = getActiveFont();
	if(!font)
		return 0;

	if(WordWrap)
	{
		s32 widest = 0;

		for(u32 line = 0; line < BrokenText.size(); ++line)
		{
			s32 width = font->getDimension(BrokenText[line].c_str()).Width;

			if(width > widest)
				widest = width;
		}

		return widest;
	}
	else
	{
		return font->getDimension(Text.c_str()).Width;
	}
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
	out->addBool	("RightToLeft",         RightToLeft);
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
	RightToLeft = in->getAttributeAsBool("RightToLeft");
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

