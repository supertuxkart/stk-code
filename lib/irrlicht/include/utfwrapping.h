// Copyright (C) 2015 Ben Au
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

namespace irr
{
namespace gui
{

//Here a list of characters that don't start or end a line for chinese/japanese/korean
//Only commonly use and full width characters are included
//You should use full width characters when writing CJK, like using "。"instead of a "."
//You can add more characters if needed
//For full list please visit http://webapp.docx4java.org/OnlineDemo/ecma376/WordML/kinsoku.html

bool UtfNoStarting (wchar_t c)
{
	switch (c)
		{
			case 8217:	//’
				return true;
			case 8221:	//”
				return true;
			case 12293:	//々
				return true;
			case 12297:	//〉
				return true;
			case 12299:	//》
				return true;
			case 12301:	//」
				return true;
			case 65373:	//｝
				return true;
			case 12309:	//〕
				return true;
			case 65289:	//）
				return true;
			case 12303:	//』
				return true;
			case 12305:	//】
				return true;
			case 12311:	//〗
				return true;
			case 65281:	//！
				return true;
			case 65285:	//％
				return true;
			case 65311:	//？
				return true;
			case 65344:	//｀
				return true;
			case 65292:	//，
				return true;
			case 65306:	//：
				return true;
			case 65307:	//；
				return true;
			case 65294:	//．
				return true;
			case 12290:	//。
				return true;
			case 12289:	//、
				return true;
			default:
			   return false;
	}
}

bool UtfNoEnding (wchar_t c)
{
	switch (c)
		{
			case 8216:	//‘
				return true;
			case 8220:	//“
				return true;
			case 12296:	//〈
				return true;
			case 12298:	//《
				return true;
			case 12300:	//「
				return true;
			case 65371:	//｛
				return true;
			case 12308:	//〔
				return true;
			case 65288:	//（
				return true;
			case 12302:	//『
				return true;
			case 12304:	//【
				return true;
			case 12310:	//〖
				return true;
			default:
				return false;
	}
}

//Helper function

bool breakable (wchar_t c)
{
	if ((c > 12287 && c < 40960) || //Common CJK words
		(c > 44031 && c < 55204)  || //Hangul
		(c > 63743 && c < 64256)  || //More Chinese
		c == 173 || c == L' ' || //Soft hyphen and white space
		c == 47 || c == 92) //Slash and blackslash
		return true;
	return false;
}
} // end namespace gui
} // end namespace irr
