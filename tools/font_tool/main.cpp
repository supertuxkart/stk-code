/*
	Tool for creating Irrlicht bitmap+vector fonts,
	started by Gaz Davidson in December 2006

	Due to my laziness and Microsoft's unituitive API, surragate pairs and
	nonspacing diacritical marks are not supported!

	Linux bitmap font support added by Neil Burlock Oct 2008
	Note: Xft/Freetype2 is used to render the fonts under X11.  Anti-aliasing
	is controlled by the system and cannot be overriden by an application,
	so fonts that are rendered will be aliased or anti-aliased depending
	on the system that they are created on.

        acme_pjz modified the original irrlicht font tool to only write the
        characters needed in a list of .po files - which saves huge amount
        of space for Asian fonts.

*/


#include <irrlicht.h>
#include <iostream>

#include "CFontTool.h"
#include "CVectorFontTool.h"
#include "ITexture.h"

using namespace irr;
using namespace gui;

#if DEBUG
bool GLContextDebugBit = true;
#else
bool GLContextDebugBit = false;
#endif

const s32 texturesizes[] = {128, 256, 512, 1024, 2048, 4096, 0};

const wchar_t *fileformats[]		 =  { L"bmp", L"ppm", 0 };  // bitmap font formats
const wchar_t *alphafileformats[]  =  { L"png", L"tga", 0 };  // bitmap font formats which support alpha channels
const wchar_t *vectorfileformats[] =  { L"xml", L"bin", 0 };  // file formats for vector fonts

const wchar_t *warntext = L"Legal Notice\n"
					L"------------\n\n"
					L"When making bitmap and vector fonts, you should consider the potential legal "
					L"issues with redistributing the fonts with your software; this tool basically "
					L"copies font data and some authors might not like this!\n"
					L"If you purchased fonts or they came with an application or your OS, you'll have"
					L"to check the license to see what restrictions are placed on making derivative works.\n\n"
					L"PD and the OFL\n"
					L"--------------\n\n"
					L"You'll find lots of fonts on the web listed as Public Domain, you can do what you like "
					L"with these.\n"
					L"Many fonts are released under the Open Font License, which is a 'viral' open source "
					L"license like the GPL. It's worth reading the license here: http://scripts.sil.org/OFL\n"
					L"The most important restrictions are- you must include the original font's license, you "
					L"can't stop your users from using or distributing the font, the font must have a "
					L"different name the original.\n\n"
					L"Some free fonts can be found here- www.openfontlibrary.org\n"
					L"http://savannah.nongnu.org/projects/freefont/";

const wchar_t *helptext = L"This tool creates bitmap fonts for the Irrlicht Engine\n\n"

					L"First select a character encoding from the list, then choose the font, "
					L"size, and whether you'd like bold, italic, antialiasing and an alpha channel. "
					L"In Windows, antialiasing will be ignored for small fonts\n\n"

					L"Then select a texture width and height. If the output exceeds this then more than "
					L"one image will be created. You can use the scrollbar at the top of the screen to "
					L"preview them. Most modern graphics cards will support up to 2048x2048, "
					L"keep in mind that more images means worse performance when drawing text!\n\n"

					L"If you want a vector font rather than a bitmap font, check the vector box. "
					L"Vector fonts are stored in system memory while bitmap fonts are in video ram\n\n"

					L"Click create to preview your font, this may take lots of time and memory "
					L"when making a font with a lot of characters, please be patient!\n\n"

					L"Now you're ready to give your font a name, select a format and click save.\n\n"
					L"To load your font in Irrlicht, simply use env->getFont(\"Myfont.xml\");\n\n"

					L"That's all, have fun :-)";

#ifdef _IRR_WINDOWS
					const wchar_t *completeText = L"Font created"
#else
					const wchar_t *completeText = L"Font created\n\n"
							L"Please note that anti-aliasing under X11 is controlled by the system "
							L"configuration, so if your system is set to use anti-aliasing, then so "
							L"will any fonts you create with FontTool";
#endif

enum MYGUI
{
	MYGUI_CHARSET  = 100,
	MYGUI_FONTNAME,
	MYGUI_SIZE,
	MYGUI_TEXWIDTH,
	MYGUI_TEXHEIGHT,
	MYGUI_BOLD,
	MYGUI_ITALIC,
	MYGUI_ANTIALIAS,
	MYGUI_ALPHA,
	MYGUI_VECTOR,
	MYGUI_FILENAME,
	MYGUI_FORMAT,
	MYGUI_CREATE,
	MYGUI_SAVE,
	MYGUI_IMAGE,
	MYGUI_CURRENTIMAGE,
	MYGUI_HELPBUTTON
};


// event reciever
class MyEventReceiver : public IEventReceiver
{
public:

	MyEventReceiver(IrrlichtDevice* device, CFontTool*& fonttool, CVectorFontTool* &vectool) :
		Device(device), FontTool(fonttool), VecTool(vectool)
	{
	}

	virtual bool OnEvent(const SEvent &event)
	{
		if (event.EventType == EET_GUI_EVENT)
		{
			s32 id = event.GUIEvent.Caller->getID();
			IGUIEnvironment* env = Device->getGUIEnvironment();

			switch(event.GUIEvent.EventType)
			{
			case EGET_SCROLL_BAR_CHANGED:
				if (id == MYGUI_CURRENTIMAGE)
				{
					IGUIImage* img = (IGUIImage*)env->getRootGUIElement()->getElementFromId(MYGUI_IMAGE,true);
					s32 i = ((IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
					img->setImage(FontTool->currentTextures[i]);

					return true;
				}
				break;
			case EGET_COMBO_BOX_CHANGED:
				if (id == MYGUI_CHARSET)
				{
					IGUIComboBox* cbo = (IGUIComboBox*)event.GUIEvent.Caller;
					IGUIComboBox* cbo_1 = (IGUIComboBox*)env->getRootGUIElement()->getElementFromId(MYGUI_FONTNAME,true);
					core::stringw str=FontTool->FontNames[cbo_1->getSelected()];
					u32 j=0;
					FontTool->selectCharSet(cbo->getSelected());
					// rebuild font list
					cbo = cbo_1;
					cbo->clear();
					for (u32 i=0; i < FontTool->FontNames.size(); ++i){
						cbo->addItem(FontTool->FontNames[i].c_str());
						if(FontTool->FontNames[i]==str) j=i;
					}
					cbo->setSelected(j);
					return true;
				}
				else if(id==MYGUI_FONTNAME){
					IGUIComboBox* cbo = (IGUIComboBox*)event.GUIEvent.Caller;
					std::cout << FontTool->FontNames[cbo->getSelected()].c_str() << std::endl;
				}
				break;
			case EGET_CHECKBOX_CHANGED:
				if (id == MYGUI_VECTOR)
				{
					IGUICheckBox* chk = (IGUICheckBox*)event.GUIEvent.Caller;

					IGUIComboBox *cbo = (IGUIComboBox*)env->getRootGUIElement()->getElementFromId(MYGUI_FORMAT,true);
					cbo->clear();

					if (chk->isChecked() && VecTool)
					{
						// vector formats
						for (s32 i=0; fileformats[i] != 0; ++i)
							cbo->addItem( core::stringw(vectorfileformats[i]).c_str());

					}
					else
					{

						// bitmap formats
						if (!FontTool->UseAlphaChannel)
						{
							// add non-alpha formats
							for (s32 i=0; fileformats[i] != 0; ++i)
								cbo->addItem( core::stringw(fileformats[i]).c_str());
						}
						// add formats which support alpha
						for (s32 i=0; alphafileformats[i] != 0; ++i)
							cbo->addItem( core::stringw(alphafileformats[i]).c_str());
					}

				}
				break;

			case EGET_BUTTON_CLICKED:

				if (id == MYGUI_CREATE)
				{
					// create the font with the params
					IGUIComboBox* cbo = (IGUIComboBox*)env->getRootGUIElement()->getElementFromId(MYGUI_CHARSET, true);
					int charset = cbo->getSelected();

					cbo = (IGUIComboBox*)env->getRootGUIElement()->getElementFromId(MYGUI_FONTNAME,true);
					int fontname = cbo->getSelected();

					/*
					cbo = (IGUIComboBox*)env->getRootGUIElement()->getElementFromId(MYGUI_SIZE,true);
					int fontsize = cbo->getSelected();
					*/

					int fontsize=wcstol(((IGUIEditBox*)env->getRootGUIElement()->getElementFromId(MYGUI_SIZE,true))->getText(),NULL,10);

					cbo = (IGUIComboBox*)env->getRootGUIElement()->getElementFromId(MYGUI_TEXWIDTH,true);
					int texwidth = cbo->getSelected();

					cbo = (IGUIComboBox*)env->getRootGUIElement()->getElementFromId(MYGUI_TEXHEIGHT,true);
					int texheight = cbo->getSelected();

					IGUICheckBox* chk = (IGUICheckBox*)env->getRootGUIElement()->getElementFromId(MYGUI_BOLD,true);
					bool bold = chk->isChecked();
					chk = (IGUICheckBox*)env->getRootGUIElement()->getElementFromId(MYGUI_ITALIC,true);
					bool italic = chk->isChecked();

					chk = (IGUICheckBox*)env->getRootGUIElement()->getElementFromId(MYGUI_ALPHA,true);
					bool alpha = chk->isChecked();

					chk = (IGUICheckBox*)env->getRootGUIElement()->getElementFromId(MYGUI_ANTIALIAS,true);
					bool aa = chk->isChecked();

					chk = (IGUICheckBox*)env->getRootGUIElement()->getElementFromId(201,true);
					bool usedOnly = chk->isChecked();

					chk = (IGUICheckBox*)env->getRootGUIElement()->getElementFromId(202,true);
					bool excludeLatin = chk->isChecked();

					// vector fonts disabled
					//chk = (IGUICheckBox*)env->getRootGUIElement()->getElementFromId(MYGUI_VECTOR,true);
					bool vec = false;//chk->isChecked();

					FontTool->makeBitmapFont(fontname, charset, /*FontTool->FontSizes[fontsize]*/ fontsize, texturesizes[texwidth], texturesizes[texheight], bold, italic, aa, alpha, usedOnly, excludeLatin);

					IGUIScrollBar* scrl = (IGUIScrollBar*)env->getRootGUIElement()->getElementFromId(MYGUI_CURRENTIMAGE,true);
					scrl->setMax(FontTool->currentTextures.size() == 0 ? 0 : FontTool->currentTextures.size()-1);

					if (FontTool->currentTextures.size() > 0)
					{
						IGUIImage* img = (IGUIImage*)env->getRootGUIElement()->getElementFromId(MYGUI_IMAGE,true);
						img->setImage(FontTool->currentTextures[0]);
						scrl->setPos(0);
					}

					// make sure users pick a file format that supports alpha channel
					cbo = (IGUIComboBox*)env->getRootGUIElement()->getElementFromId(MYGUI_FORMAT,true);
					cbo->clear();

					if (vec)
					{
						// add vector formats
						for (s32 i=0; fileformats[i] != 0; ++i)
							cbo->addItem( core::stringw(vectorfileformats[i]).c_str());
					}
					else
					{
						if (!alpha)
						{
							// add non-alpha formats
							for (s32 i=0; fileformats[i] != 0; ++i)
								cbo->addItem( core::stringw(fileformats[i]).c_str());
						}
						// add formats which support alpha
						for (s32 i=0; alphafileformats[i] != 0; ++i)
							cbo->addItem( core::stringw(alphafileformats[i]).c_str());
					}
					if (VecTool)
					{
						delete VecTool;
						VecTool = 0;
					}
					if (vec)
					{
						VecTool = new CVectorFontTool(FontTool);
					}

					/* Message box letting the user know the process is complete */
					env->addMessageBox(L"Create", completeText);

					return true;
				}

				if (id == MYGUI_SAVE)
				{
					IGUIEditBox *edt  = (IGUIEditBox*)env->getRootGUIElement()->getElementFromId(MYGUI_FILENAME,true);
					core::stringc name = edt->getText();

					IGUIComboBox *fmt  = (IGUIComboBox*)env->getRootGUIElement()->getElementFromId(MYGUI_FORMAT,true);
					core::stringc format = fmt->getItem(fmt->getSelected());

					// vector fonts disabled
					// IGUICheckBox *chk = (IGUICheckBox*)env->getRootGUIElement()->getElementFromId(MYGUI_VECTOR,true);
					// bool vec = chk->isChecked();
					bool vec = false;

					if (vec && VecTool)
						VecTool->saveVectorFont(name.c_str(), format.c_str());
					else
						FontTool->saveBitmapFont(name.c_str(), format.c_str());

					return true;
				}

				if (id == MYGUI_HELPBUTTON)
				{
					env->addMessageBox(L"Irrlicht Unicode Font Tool", helptext);
					return true;
				}

				break;

			default:
				break;
			}
		}

		return false;
	}

	IrrlichtDevice*	 Device;
	CFontTool*       FontTool;
	CVectorFontTool* VecTool;

};

void createGUI(IrrlichtDevice* device, CFontTool* fc)
{
	gui::IGUIEnvironment *env = device->getGUIEnvironment();

	// change transparency of skin
	for (s32 i=0; i<gui::EGDC_COUNT ; ++i)
	{
		video::SColor col = env->getSkin()->getColor((gui::EGUI_DEFAULT_COLOR)i);
		col.setAlpha(255);
		env->getSkin()->setColor((gui::EGUI_DEFAULT_COLOR)i, col);
	}

	IGUIWindow *win = env->addWindow( core::rect<s32>(10,10,200,500), false, L"Font Creator");
	win->getCloseButton()->setVisible(false);

	s32 xs=10,xp=xs, yp=30, h=20;

	env->addStaticText(L"Charset", core::rect<s32>(xp,yp,50,yp+h),false,false, win);

	xp+=60;

	// charset combo
	gui::IGUIComboBox* cbo = env->addComboBox( core::rect<s32>(xp,yp,180,yp+h),win, MYGUI_CHARSET);
	for (u32 i=0; i < fc->CharSets.size(); ++i)
		cbo->addItem(fc->CharSets[i].c_str());

	yp += (s32)(h*1.5f);
	xp = xs;

	env->addStaticText(L"Font", core::rect<s32>(xp,yp,50,yp+h),false,false, win);

	xp+=60;

	// font name combo
	cbo = env->addComboBox( core::rect<s32>(xp,yp,180,yp+h),win, MYGUI_FONTNAME);
	for (u32 i=0; i < fc->FontNames.size(); ++i){
		cbo->addItem(fc->FontNames[i].c_str());
		if(fc->FontNames[i] == L"\u6587\u6CC9\u9A7F\u5FAE\u7C73\u9ED1") cbo->setSelected(i); //auto select wqy-microhei
	}

	yp += (s32)(h*1.5f);
	xp = xs;

	env->addStaticText(L"Size", core::rect<s32>(xp,yp,50,yp+h),false,false, win);

	xp += 60;

	/*
	// font size combo
	cbo = env->addComboBox( core::rect<s32>(xp,yp,xp+50,yp+h),win, MYGUI_SIZE);
	for (s32 i=0; fc->FontSizes[i] != 0; ++i)
		cbo->addItem( ((core::stringw(fc->FontSizes[i])) + L"pt").c_str()); */

	env->addEditBox(L"4",core::rect<s32>(xp,yp,xp+70,yp+h), true, win, MYGUI_SIZE);
	xp += 80;
	env->addStaticText(L"pt", core::rect<s32>(xp,yp,xp+50,yp+h),false,false,win);

	xp = xs;
	yp += (s32)(h*1.5f);

	// bold checkbox
	env->addCheckBox(false, core::rect<s32>(xp,yp,xp+50,yp+h),win, MYGUI_BOLD, L"Bold");

	xp += 45;

	// italic checkbox
	env->addCheckBox(false, core::rect<s32>(xp,yp,xp+50,yp+h),win, MYGUI_ITALIC, L"Italic");

	xp += 45;

	// AA checkbox
	env->addCheckBox(false, core::rect<s32>(xp,yp,xp+50,yp+h),win, MYGUI_ANTIALIAS, L"AA");

	xp +=40;

	// Alpha checkbox
	env->addCheckBox(false, core::rect<s32>(xp,yp,xp+50,yp+h),win, MYGUI_ALPHA, L"Alpha");

	xp = xs;
	yp += (s32)(h*1.5f);

	//new

	env->addCheckBox(false, core::rect<s32>(xp,yp,xp+150,yp+h),win, 201, L"Export used characters only")->setChecked(false);
	yp += (s32)(h*1.5f);

	env->addCheckBox(false, core::rect<s32>(xp,yp,xp+150,yp+h),win, 202, L"Exclude basic latin characters");
	yp += (s32)(h*1.5f);

	/*
	// vector fonts can't be loaded yet
	env->addCheckBox(false, core::rect<s32>(xp,yp,xp+200,yp+h),win, MYGUI_VECTOR, L"Vector Font");

	yp += (s32)(h*1.5f);
	*/

	env->addStaticText(L"Max Width:", core::rect<s32>(xp,yp,50,yp+h),false,false, win);

	xp += 60;

	// texture widths
	cbo = env->addComboBox( core::rect<s32>(xp,yp,xp+70,yp+h),win, MYGUI_TEXWIDTH);
	for (s32 i=0; texturesizes[i] != 0; ++i)
		cbo->addItem( ((core::stringw(texturesizes[i])) + L" wide").c_str());

	xp=xs;
	yp += (s32)(h*1.5f);

	env->addStaticText(L"Max Height:", core::rect<s32>(xp,yp,60,yp+h),false,false, win);

	xp += 60;

	// texture height
	cbo = env->addComboBox( core::rect<s32>(xp,yp,xp+70,yp+h),win, MYGUI_TEXHEIGHT);
	for (s32 i=0; texturesizes[i] != 0; ++i)
		cbo->addItem( ((core::stringw(texturesizes[i])) + L" tall").c_str());

	// file name
	xp = xs;
	yp += (s32)(h*1.5f);
	env->addStaticText(L"Filename", core::rect<s32>(xp,yp,60,yp+h),false,false, win);
	xp += 60;
	env->addEditBox(L"myfont",core::rect<s32>(xp,yp,xp+70,yp+h), true, win, MYGUI_FILENAME);

	// file format
	xp = xs;
	yp += (s32)(h*1.5f);
	env->addStaticText(L"File Format", core::rect<s32>(xp,yp,60,yp+h),false,false, win);
	xp += 60;

	cbo = env->addComboBox( core::rect<s32>(xp,yp,xp+70,yp+h),win, MYGUI_FORMAT);
	for (s32 i=0; fileformats[i] != 0; ++i)
		cbo->addItem( core::stringw(fileformats[i]).c_str());
	for (s32 i=0; alphafileformats[i] != 0; ++i)
		cbo->addItem( core::stringw(alphafileformats[i]).c_str());

	xp = xs;
	yp += h*2;

	// create button
	env->addButton( core::rect<s32>(xp,yp,xp+50,yp+h),win, MYGUI_CREATE, L"Create");

	xp += 60;

	// save button
	env->addButton( core::rect<s32>(xp,yp,xp+50,yp+h),win, MYGUI_SAVE, L"Save");

	xp += 60;

	// help button
	env->addButton( core::rect<s32>(xp,yp,xp+50,yp+h),win, MYGUI_HELPBUTTON, L"Help");

	// font image
	gui::IGUIImage *img = env->addImage(0, core::position2d<s32>(0,0), true,0, MYGUI_IMAGE);
	img->setRelativePosition(core::rect<s32>(0,20,800,600));

	// font scrollbar
	IGUIScrollBar *scrl= env->addScrollBar(true,core::rect<s32>(0,0,800,20),0, MYGUI_CURRENTIMAGE);
	scrl->setMax(0);
	scrl->setSmallStep(1);

	yp += h*3;

	env->getRootGUIElement()->bringToFront(win);
	win->setRelativePosition( core::rect<s32>(10,10,200,yp));
}

int main(int argc,char **argv)
{
	IrrlichtDevice* device =createDevice(video::EDT_OPENGL, core::dimension2du(800, 600));
	video::IVideoDriver* driver = device->getVideoDriver();
	scene::ISceneManager* smgr = device->getSceneManager();
	gui::IGUIEnvironment *env = device->getGUIEnvironment();

	// create font tool
	CFontTool *fc = new CFontTool(device);
	CVectorFontTool *vc = 0;

	MyEventReceiver events(device,fc,vc);
	device->setEventReceiver(&events);

	createGUI(device, fc);

    for(int i=1; i<argc; i++)
    {
        if(!strcmp(argv[i],"-c") && i<argc-1)
        {
            i++;
            if(setUsedCharacters(argv[i]))
            {
                IGUICheckBox *box =
                    dynamic_cast<IGUICheckBox*>(device->getGUIEnvironment()
                                                     ->getRootGUIElement()
                                                     ->getElementFromId(201, true));
                box->setChecked(true);
            }

        }
        else
        {  
            // Old style: just a single parameter, assume it's a file name with pot files in it
            if(LoadPoFiles(argv[i]))
            {
                IGUICheckBox *box =
                    dynamic_cast<IGUICheckBox*>(device->getGUIEnvironment()
                    ->getRootGUIElement()
                    ->getElementFromId(201, true));
                box->setChecked(true);
            }
        }
    }   // for i <argc

	while(device->run())
	{
		device->sleep(50);
		if (device->isWindowActive())
		{

			driver->beginScene(true, true, video::SColor(0,200,200,200));
			smgr->drawAll();
			env->drawAll();
			driver->endScene();
		}
	}

	// drop the font tool and resources
	fc->drop();

	device->drop();

	return 0;
}

