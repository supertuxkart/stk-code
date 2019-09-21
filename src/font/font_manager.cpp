//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2016 SuperTuxKart-Team
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

#include "font/font_manager.hpp"

#include "io/file_manager.hpp"
#include "font/bold_face.hpp"
#include "font/digit_face.hpp"
#include "font/face_ttf.hpp"
#include "font/regular_face.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/skin.hpp"
#include "modes/profile_world.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#ifndef SERVER_ONLY
#include <fribidi/fribidi.h>
#include <harfbuzz/hb.h>
#include <raqm.h>
#endif

FontManager *font_manager = NULL;
// ----------------------------------------------------------------------------
/** Constructor. It will initialize the \ref m_ft_library.
 */
FontManager::FontManager()
{
#ifndef SERVER_ONLY
    m_has_color_emoji = false;
    m_ft_library = NULL;
    m_digit_face = NULL;
    m_shaping_dpi = 128;
    if (ProfileWorld::isNoGraphics())
        return;

    checkFTError(FT_Init_FreeType(&m_ft_library), "loading freetype library");
#endif
}   // FontManager

// ----------------------------------------------------------------------------
/** Destructor. Clears all fonts and related stuff.
 */
FontManager::~FontManager()
{
    for (unsigned int i = 0; i < m_fonts.size(); i++)
        delete m_fonts[i];
    m_fonts.clear();

#ifndef SERVER_ONLY
    if (ProfileWorld::isNoGraphics())
        return;

    for (unsigned int i = 0; i < m_faces.size(); i++)
        checkFTError(FT_Done_Face(m_faces[i]), "removing faces for shaping");
    if (m_digit_face != NULL)
        checkFTError(FT_Done_Face(m_digit_face), "removing digit face");
    checkFTError(FT_Done_FreeType(m_ft_library), "removing freetype library");
#endif
}   // ~FontManager

#ifndef SERVER_ONLY
// ----------------------------------------------------------------------------
/** Load all TTFs from a list to m_faces.
 *  \param ttf_list List of TTFs to be loaded.
 */
std::vector<FT_Face>
                 FontManager::loadTTF(const std::vector<std::string>& ttf_list)
{
    std::vector <FT_Face> ret;
    if (ProfileWorld::isNoGraphics())
        return ret;

    for (const std::string& font : ttf_list)
    {
        FT_Face face = NULL;
        font_manager->checkFTError(FT_New_Face(
            m_ft_library, font.c_str(), 0, &face), font + " is loaded");
        ret.push_back(face);
    }
    return ret;
}   // loadTTF

// ============================================================================
namespace LineBreakingRules
{
    // Here a list of characters that don't start or end a line for
    // chinese/japanese/korean. Only commonly use and full width characters are
    // included. You should use full width characters when writing CJK,
    // like using "。"instead of a ".", you can add more characters if needed.
    // For full list please visit:
    // http://webapp.docx4java.org/OnlineDemo/ecma376/WordML/kinsoku.html
    bool noStartingLine(char32_t c)
    {
        switch (c)
        {
            // ’
            case 8217:
                return true;
            // ”
            case 8221:
                return true;
            // 々
            case 12293:
                return true;
            // 〉
            case 12297:
                return true;
            // 》
            case 12299:
                return true;
            // 」
            case 12301:
                return true;
            // ｝
            case 65373:
                return true;
            // 〕
            case 12309:
                return true;
            // ）
            case 65289:
                return true;
            // 』
            case 12303:
                return true;
            // 】
            case 12305:
                return true;
            // 〗
            case 12311:
                return true;
            // ！
            case 65281:
                return true;
            // ％
            case 65285:
                return true;
            // ？
            case 65311:
                return true;
            // ｀
            case 65344:
                return true;
            // ，
            case 65292:
                return true;
            // ：
            case 65306:
                return true;
            // ；
            case 65307:
                return true;
            // ．
            case 65294:
                return true;
            // 。
            case 12290:
                return true;
            // 、
            case 12289:
                return true;
            default:
                return false;
        }
    }   // noStartingLine
    //-------------------------------------------------------------------------
    bool noEndingLine(char32_t c)
    {
        switch (c)
        {
            // ‘
            case 8216:
                return true;
            // “
            case 8220:
                return true;
            // 〈
            case 12296:
                return true;
            // 《
            case 12298:
                return true;
            // 「
            case 12300:
                return true;
            // ｛
            case 65371:
                return true;
            // 〔
            case 12308:
                return true;
            // （
            case 65288:
                return true;
            // 『
            case 12302:
                return true;
            // 【
            case 12304:
                return true;
            // 〖
            case 12310:
                return true;
            default:
                return false;
        }
    }   // noEndingLine
    //-------------------------------------------------------------------------
    // Helper function
    bool breakable(char32_t c)
    {
        if ((c > 12287 && c < 40960) || // Common CJK words
            (c > 44031 && c < 55204)  || // Hangul
            (c > 63743 && c < 64256)  || // More Chinese
            c == 173 || c == 32 || // Soft hyphen and white space
            c == 47 || c == 92 || // Slash and blackslash
            c == 8203) // Zero-width space
            return true;
        return false;
    }   // breakable
    //-------------------------------------------------------------------------
    void insertBreakMark(const std::u32string& str, std::vector<bool>& result)
    {
        assert(str.size() == result.size());
        for (unsigned i = 0; i < result.size(); i++)
        {
            char32_t c = str[i];
            char32_t nextline_char = 20;
            if (i < result.size() - 1)
                nextline_char = str[i + 1];
            if (breakable(c) && !noEndingLine(c) &&
                !noStartingLine(nextline_char))
            {
                result[i] = true;
            }
        }
    }   // insertBreakMark
}   // namespace LineBreakingRules

// ============================================================================
namespace RTLRules
{
    //-------------------------------------------------------------------------
    void insertRTLMark(const std::u32string& str, std::vector<bool>& line,
                       std::vector<bool>& char_bool)
    {
        // Check if first character has strong RTL, then consider this line is
        // RTL
        std::vector<FriBidiCharType> types;
        std::vector<FriBidiLevel> levels;
        types.resize(str.size(), 0);
        levels.resize(str.size(), 0);
        FriBidiParType par_type = FRIBIDI_PAR_ON;
        const FriBidiChar* fribidi_str = (const FriBidiChar*)str.c_str();
        fribidi_get_bidi_types(fribidi_str, str.size(), types.data());
#if FRIBIDI_MAJOR_VERSION >= 1
        std::vector<FriBidiBracketType> btypes;
        btypes.resize(str.size(), 0);
        fribidi_get_bracket_types(fribidi_str, str.size(), types.data(),
            btypes.data());
        int max_level = fribidi_get_par_embedding_levels_ex(types.data(),
            btypes.data(), str.size(), &par_type, levels.data());
#else
        int max_level = fribidi_get_par_embedding_levels(types.data(),
            str.size(), &par_type, levels.data());
#endif
        (void)max_level;
        bool cur_rtl = par_type == FRIBIDI_PAR_RTL;
        if (cur_rtl)
        {
            for (unsigned i = 0; i < line.size(); i++)
                line[i] = true;
        }
        int cur_level = levels[0];
        for (unsigned i = 0; i < char_bool.size(); i++)
        {
            if (levels[i] != cur_level)
            {
                cur_rtl = !cur_rtl;
                cur_level = levels[i];
            }
            char_bool[i] = cur_rtl;
        }
    }   // insertRTLMark
}   // namespace RTLRules

// ----------------------------------------------------------------------------
/* Turn text into glyph layout for rendering by libraqm. */
void FontManager::shape(const std::u32string& text,
                        std::vector<irr::gui::GlyphLayout>& gls,
                        std::vector<std::u32string>* line_data)

{
    // m_faces can be empty in null device
    if (text.empty() || m_faces.empty())
        return;

    auto lines = StringUtils::split(text, U'\n');
    // If the text end with and newline, it will miss a newline height, so we
    // it back here
    if (text.back() == U'\n')
        lines.push_back(U"");

    for (unsigned l = 0; l < lines.size(); l++)
    {
        if (l != 0)
        {
            gui::GlyphLayout gl = { 0 };
            gl.flags = gui::GLF_NEWLINE;
            gls.push_back(gl);
        }

        std::u32string& str = lines[l];
        str.erase(std::remove(str.begin(), str.end(), U'\r'), str.end());
        str.erase(std::remove(str.begin(), str.end(), U'\t'), str.end());
        if (str.empty())
        {
            if (line_data)
                line_data->push_back(str);
            continue;
        }

        raqm_t* rq = raqm_create();
        if (!rq)
        {
            Log::error("FontManager", "Failed to raqm_create.");
            gls.clear();
            if (line_data)
                line_data->clear();
            return;
        }

        const int length = (int)str.size();
        const uint32_t* string_array = (const uint32_t*)str.c_str();

        if (!raqm_set_text(rq, string_array, length))
        {
            Log::error("FontManager", "Failed to raqm_set_text.");
            raqm_destroy(rq);
            gls.clear();
            if (line_data)
                line_data->clear();
            return;
        }

        FT_Face prev_face = NULL;
        for (int i = 0; i < length; i++)
        {
            FT_Face cur_face = m_faces.front();
            bool override_face = false;
            if (prev_face != NULL && i != 0)
            {
                hb_script_t prev_script = hb_unicode_script(
                    hb_unicode_funcs_get_default(), str[i - 1]);
                hb_script_t cur_script = hb_unicode_script(
                    hb_unicode_funcs_get_default(), str[i]);
                if (cur_script == HB_SCRIPT_INHERITED ||
                    (prev_script == HB_SCRIPT_ARABIC &&
                    // Those exists in the default arabic font
                    (str[i] == U'.' || str[i] == U'!' || str[i] == U':')))
                {
                    // For inherited script (like punctation with arabic or
                    // join marks), try to use the previous face so it is not
                    // hb_shape separately
                    cur_face = prev_face;
                    override_face = true;
                }
            }
            FT_Face emoji_face = m_faces.size() > 1 ? m_faces[1] : NULL;
            if (m_has_color_emoji && !override_face &&
                length > 1 && i < length - 1 &&
                emoji_face != NULL && str[i + 1] == 0xfe0f)
            {
                // Rule for variation selector-16 (emoji presentation)
                // It is used in for example Keycap Digit One
                // (U+31, U+FE0F, U+20E3)
                cur_face = emoji_face;
                override_face = true;
            }
            if (!override_face)
            {
                for (unsigned j = 0; j < m_faces.size(); j++)
                {
                    unsigned glyph_index =
                        FT_Get_Char_Index(m_faces[j], str[i]);
                    if (glyph_index > 0)
                    {
                        cur_face = m_faces[j];
                        break;
                    }
                }
            }
            prev_face = cur_face;
            if (!FT_HAS_COLOR(cur_face) ||
                (FT_HAS_COLOR(cur_face) && cur_face->num_fixed_sizes == 0))
            {
                // Handle color emoji with CPAL / COLR tables
                // (num_fixed_sizes == 0)
                checkFTError(FT_Set_Pixel_Sizes(cur_face, 0,
                    m_shaping_dpi), "setting DPI");
            }
            if (!raqm_set_freetype_face_range(rq, cur_face, i, 1))
            {
                Log::error("FontManager",
                    "Failed to raqm_set_freetype_face_range.");
                raqm_destroy(rq);
                gls.clear();
                if (line_data)
                    line_data->clear();
                return;
            }
        }

        if (raqm_layout(rq))
        {
            std::vector<gui::GlyphLayout> cur_line;
            std::vector<bool> rtl_line, rtl_char, breakable;
            rtl_line.resize(str.size(), false);
            rtl_char.resize(str.size(), false);
            breakable.resize(str.size(), false);
            LineBreakingRules::insertBreakMark(str, breakable);
            translations->insertThaiBreakMark(str, breakable);
            RTLRules::insertRTLMark(str, rtl_line, rtl_char);
            size_t count = 0;
            raqm_glyph_t* glyphs = raqm_get_glyphs(rq, &count);
            for (unsigned idx = 0; idx < (unsigned)count; idx++)
            {
                gui::GlyphLayout gl = { 0 };
                gl.index = glyphs[idx].index;
                gl.cluster.push_back(glyphs[idx].cluster);
                gl.x_advance = glyphs[idx].x_advance / BEARING;
                gl.x_offset = glyphs[idx].x_offset / BEARING;
                gl.y_offset = glyphs[idx].y_offset / BEARING;
                gl.face_idx = m_ft_faces_to_index.at(glyphs[idx].ftface);
                gl.original_index = idx;
                if (rtl_line[glyphs[idx].cluster])
                    gl.flags |= gui::GLF_RTL_LINE;
                if (rtl_char[glyphs[idx].cluster])
                    gl.flags |= gui::GLF_RTL_CHAR;
                if (FT_HAS_COLOR(glyphs[idx].ftface))
                    gl.flags |= gui::GLF_COLORED;
                cur_line.push_back(gl);
            }
            // Sort glyphs in logical order
            std::sort(cur_line.begin(), cur_line.end(), []
                (const gui::GlyphLayout& a_gi, const gui::GlyphLayout& b_gi)
                {
                    return a_gi.cluster.front() < b_gi.cluster.front();
                });
            for (unsigned l = 0; l < cur_line.size(); l++)
            {
                const int cur_cluster = cur_line[l].cluster.front();
                // Last cluster handling, add the remaining clusters if missing
                if (l == cur_line.size() - 1)
                {
                    for (int extra_cluster = cur_cluster + 1;
                        extra_cluster <= (int)str.size() - 1; extra_cluster++)
                        cur_line[l].cluster.push_back(extra_cluster);
                }
                else
                {
                    // Make sure there is every cluster values appear in the
                    // list at least once, it will be used for cursor
                    // positioning later
                    const int next_cluster = cur_line[l + 1].cluster.front();
                    for (int extra_cluster = cur_cluster + 1;
                        extra_cluster <= next_cluster - 1; extra_cluster++)
                        cur_line[l].cluster.push_back(extra_cluster);
                }
                cur_line[l].draw_flags.resize(cur_line[l].cluster.size(),
                    gui::GLD_NONE);
            }
            // Sort glyphs in visual order
            std::sort(cur_line.begin(), cur_line.end(), []
                (const gui::GlyphLayout& a_gi,
                const gui::GlyphLayout& b_gi)
                {
                    return a_gi.original_index < b_gi.original_index;
                });
            // Use last cluster to determine link breaking, so ligatures can be
            // handled
            for (gui::GlyphLayout& gl : cur_line)
            {
                int last_cluster = gl.cluster.back();
                if (breakable[last_cluster])
                    gl.flags |= gui::GLF_BREAKABLE;
            }
            gls.insert(gls.end(), cur_line.begin(), cur_line.end());
            raqm_destroy(rq);
            if (line_data)
                line_data->push_back(str);
        }
        else
        {
            Log::error("FontManager", "Failed to raqm_layout.");
            raqm_destroy(rq);
            gls.clear();
            if (line_data)
                line_data->clear();
            return;
        }
    }
}   // shape

// ----------------------------------------------------------------------------
/* Return the cached glyph layouts for writing, it will clear all layouts if
 * not in-game and when the cached sized exceed a certain number. */
std::vector<irr::gui::GlyphLayout>&
                   FontManager::getCachedLayouts(const irr::core::stringw& str)
{
    const size_t MAX_LAYOUTS = 600;
    if (StateManager::get()->getGameState() != GUIEngine::GAME &&
        m_cached_gls.size() > MAX_LAYOUTS)
    {
        Log::debug("FontManager",
            "Clearing cached glyph layouts because too many.");
        clearCachedLayouts();
    }
    return m_cached_gls[str];
}   // getCachedLayouts

// ----------------------------------------------------------------------------
/** Convert text to glyph layouts for fast rendering with caching enabled
 *  If line_data is not null, each broken line u32string will be saved and
 *  can be used for advanced glyph and text mapping, and cache will be
 *  disabled, no newline characters are allowed in text if line_data is not
 *  NULL.
 */
void FontManager::initGlyphLayouts(const core::stringw& text,
                                   std::vector<irr::gui::GlyphLayout>& gls,
                                   std::vector<std::u32string>* line_data)
{
    if (ProfileWorld::isNoGraphics() || text.empty())
        return;

    if (line_data != NULL)
    {
        shape(StringUtils::wideToUtf32(text), gls, line_data);
        return;
    }

    auto& cached_gls = getCachedLayouts(text);
    if (cached_gls.empty())
        shape(StringUtils::wideToUtf32(text), cached_gls);
    gls = cached_gls;
}   // initGlyphLayouts

// ----------------------------------------------------------------------------
FT_Face FontManager::loadColorEmoji()
{
    if (GUIEngine::getSkin()->getColorEmojiTTF().empty())
        return NULL;
    FT_Face face = NULL;
    FT_Error err = FT_New_Face(m_ft_library,
        GUIEngine::getSkin()->getColorEmojiTTF().c_str(), 0, &face);
    if (err > 0)
    {
        Log::error("FontManager", "Something wrong when loading color emoji! "
            "The error code was %d.", err);
        return NULL;
    }

    if (!FT_HAS_COLOR(face))
    {
        Log::error("FontManager", "Bad %s color emoji, ignored.",
            GUIEngine::getSkin()->getColorEmojiTTF().c_str());
        checkFTError(FT_Done_Face(face), "removing faces for emoji");
        return NULL;
    }
    if (face->num_fixed_sizes != 0)
    {
        // Use the largest size available, it will be scaled to regular face ttf
        // when loading the glyph, so it can reduce the blurring effect
        m_shaping_dpi = face->available_sizes[face->num_fixed_sizes - 1].height;
        checkFTError(FT_Select_Size(face, face->num_fixed_sizes - 1),
            "setting color emoji size");
    }

    uint32_t smiley = 0x1f603;
    uint32_t glyph_index = FT_Get_Char_Index(face, smiley);
    if (glyph_index == 0)
    {
        Log::error("FontManager", "%s doesn't make 0x1f603 smiley, ignored.",
            GUIEngine::getSkin()->getColorEmojiTTF().c_str());
        checkFTError(FT_Done_Face(face), "removing faces for emoji");
        return NULL;
    }
    FT_GlyphSlot slot = face->glyph;
    if (FT_HAS_COLOR(face) && face->num_fixed_sizes != 0)
    {
        checkFTError(FT_Load_Glyph(face, glyph_index,
            FT_LOAD_DEFAULT | FT_LOAD_COLOR), "loading a glyph");
    }
    else
    {
        checkFTError(FT_Set_Pixel_Sizes(face, 0, 16), "setting DPI");
        checkFTError(FT_Load_Glyph(face, glyph_index,
            FT_LOAD_DEFAULT | FT_LOAD_COLOR), "loading a glyph");
        checkFTError(FT_Render_Glyph(slot,
            FT_RENDER_MODE_NORMAL), "rendering a glyph to bitmap");
    }

    FT_Bitmap* bits = &(slot->bitmap);
    if (!bits || bits->pixel_mode != FT_PIXEL_MODE_BGRA)
    {
        Log::error("FontManager", "%s doesn't have color, ignored.",
            GUIEngine::getSkin()->getColorEmojiTTF().c_str());
        checkFTError(FT_Done_Face(face), "removing faces for emoji");
        return NULL;
    }

    m_has_color_emoji = true;
    return face;
}   // loadColorEmoji

#endif

// ----------------------------------------------------------------------------
/** Initialize all \ref FaceTTF and \ref FontWithFace members.
 */
void FontManager::loadFonts()
{
#ifndef SERVER_ONLY
    // First load the TTF files required by each font
    std::vector<FT_Face> normal_ttf = loadTTF(
        GUIEngine::getSkin()->getNormalTTF());
    std::vector<FT_Face> bold_ttf = normal_ttf;
    FT_Face color_emoji = NULL;
    if (!ProfileWorld::isNoGraphics())
    {
        assert(!normal_ttf.empty());
        color_emoji = loadColorEmoji();
        if (!normal_ttf.empty() && color_emoji != NULL)
        {
            // Put color emoji after 1st default font so can use it before wqy
            // font
            normal_ttf.insert(normal_ttf.begin() + 1, color_emoji);
            // We don't use color emoji in bold font
            bold_ttf.insert(bold_ttf.begin() + 1, NULL);
        }
        // We use 16bit face idx in GlyphLayout class
        if (normal_ttf.size() > 65535)
            normal_ttf.resize(65535);
        for (uint16_t i = 0; i < normal_ttf.size(); i++)
            m_ft_faces_to_index[normal_ttf[i]] = i;
    }

    std::vector<FT_Face> digit_ttf =
        loadTTF(GUIEngine::getSkin()->getDigitTTF());
    if (!digit_ttf.empty())
        m_digit_face = digit_ttf.front();
#endif

    // Now load fonts with settings of ttf file
    unsigned int font_loaded = 0;
    RegularFace* regular = new RegularFace();
#ifndef SERVER_ONLY
    regular->getFaceTTF()->loadTTF(normal_ttf);
#endif
    regular->init();

#ifndef SERVER_ONLY
    if (color_emoji && color_emoji->num_fixed_sizes == 0)
    {
        // This color emoji has CPAL / COLR tables so it's scalable
        m_shaping_dpi = regular->getDPI();
        // Update inverse shaping from m_shaping_dpi
        regular->setDPI();
    }
#endif

    m_fonts.push_back(regular);
    m_font_type_map[std::type_index(typeid(RegularFace))] = font_loaded++;

    BoldFace* bold = new BoldFace();
#ifndef SERVER_ONLY
    bold->getFaceTTF()->loadTTF(bold_ttf);
#endif
    bold->init();
    m_fonts.push_back(bold);
    m_font_type_map[std::type_index(typeid(BoldFace))] = font_loaded++;

    DigitFace* digit = new DigitFace();
#ifndef SERVER_ONLY
    digit->getFaceTTF()->loadTTF(digit_ttf);
#endif
    digit->init();
    m_fonts.push_back(digit);
    m_font_type_map[std::type_index(typeid(DigitFace))] = font_loaded++;

#ifndef SERVER_ONLY
    m_faces.insert(m_faces.end(), normal_ttf.begin(), normal_ttf.end());
#endif
}   // loadFonts

// ----------------------------------------------------------------------------
/** Unit testing that will try to load all translations in STK, and discover if
 *  there is any characters required by it are not supported in \ref
 *  m_normal_ttf.
 */
void FontManager::unitTesting()
{
#ifndef SERVER_ONLY
    std::vector<std::string> list = *(translations->getLanguageList());
    const int cur_log_level = Log::getLogLevel();
    for (const std::string& lang : list)
    {
        // Hide gettext warning
        Log::setLogLevel(5);
        delete translations;
#ifdef WIN32
        std::string s=std::string("LANGUAGE=") + lang.c_str();
        _putenv(s.c_str());
#else
        setenv("LANGUAGE", lang.c_str(), 1);
#endif
        translations = new Translations();
        Log::setLogLevel(cur_log_level);
        std::set<wchar_t> used_chars = translations->getCurrentAllChar();
        // First FontWithFace is RegularFace
        FaceTTF* ttf = m_fonts.front()->getFaceTTF();
        for (const wchar_t& c : used_chars)
        {
            // Skip non-printing characters
            if (c < 32) continue;

            unsigned int font_number = 0;
            unsigned int glyph_index = 0;
            if (ttf->getFontAndGlyphFromChar(c, &font_number, &glyph_index))
            {
                Log::debug("UnitTest", "Character %s in language %s"
                    " use face %s",
                    StringUtils::wideToUtf8(core::stringw(&c, 1)).c_str(),
                    lang.c_str(),
                    ttf->getFace(font_number)->family_name);
            }
            else
            {
                Log::warn("UnitTest", "Character %s in language %s"
                    " is not supported by all fonts!",
                    StringUtils::wideToUtf8(core::stringw(&c, 1)).c_str(),
                    lang.c_str());
            }
        }
    }
#endif
}   // unitTesting
