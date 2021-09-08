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
#include "states_screens/state_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#ifndef SERVER_ONLY
#include <harfbuzz/hb-ft.h>
extern "C"
{
    #include <SheenBidi.h>
}
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
    m_hb_buffer = NULL;
    if (GUIEngine::isNoGraphics())
        return;

    m_hb_buffer = hb_buffer_create();
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
    if (GUIEngine::isNoGraphics())
        return;

    hb_buffer_destroy(m_hb_buffer);
    for (hb_font_t* font : m_hb_fonts)
        hb_font_destroy(font);
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
    if (GUIEngine::isNoGraphics())
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

// ----------------------------------------------------------------------------
/* Turn text into glyph layout for rendering by libraqm. */
void FontManager::shape(const std::u32string& text,
                        std::vector<irr::gui::GlyphLayout>& gls,
                        u32 shape_flag)
{
    // Helper struct
    struct ShapeGlyph
    {
        unsigned int index;
        int x_advance;
        int y_advance;
        int x_offset;
        int y_offset;
        uint32_t cluster;
        FT_Face ftface;
    };
    auto fill_shape_glyph = [](std::vector<ShapeGlyph>& shape_glyphs,
        hb_buffer_t* hb_buffer, int offset, FT_Face ftface)
    {
        size_t len = hb_buffer_get_length(hb_buffer);
        hb_glyph_info_t* info = hb_buffer_get_glyph_infos(hb_buffer, NULL);
        hb_glyph_position_t* position =
            hb_buffer_get_glyph_positions(hb_buffer, NULL);
        for (size_t i = 0; i < len; i++)
        {
            shape_glyphs.push_back({info[i].codepoint, position[i].x_advance,
                position[i].y_advance, position[i].x_offset,
                position[i].y_offset, info[i].cluster + offset, ftface});
        }
    };
    // m_faces can be empty in null device
    if (text.empty() || m_faces.empty())
        return;

    auto lines = StringUtils::split(text, U'\n');
    // If the text end with and newline, it will miss a newline height, so we
    // it back here
    if (text.back() == U'\n')
        lines.push_back(U"");

    // URL marker
    std::vector<std::pair<int, int> > http_pos;
    auto fix_end_pos = [](const std::u32string& url, size_t start_pos,
                          size_t pos)->size_t
    {
        // https:// has 8 characters, shortest URL has 3 characters (like t.me)
        // so 8 is valid for http:// too
        size_t next_forward_slash = url.find(U'/', start_pos + 8);
        if (next_forward_slash > pos)
            next_forward_slash = std::string::npos;

        // Tested in gnome terminal, URL ends with 0-9, aA-zZ, /- or ~:_=#$%&'+@*]) only
        // ~:_=#$%&'+@*]) will not be highlighted unless it's after / (forward slash)
        // We assume the URL is valid so we only test ]) instead of ([ blah ])
        std::u32string valid_end_characters = U"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789/-";
        std::u32string valid_end_characters_extra = U"~:_=#$%&'+@*])";
        if (next_forward_slash != std::string::npos)
            valid_end_characters += valid_end_characters_extra;

        while (pos > 1)
        {
            char32_t url_char = url[pos - 1];
            for (char32_t valid_char : valid_end_characters)
            {
                if (valid_char == url_char)
                    return pos;
            }
            pos--;
        }
        return 0;
    };

    // Auto URL highlighting for http:// or https://
    size_t pos = text.find(U"http://", 0);
    while (pos != std::u32string::npos)
    {
        // Find nearest newline or whitespace
        size_t newline_pos = text.find(U'\n', pos + 1);
        size_t space_pos = text.find(U' ', pos + 1);
        size_t end_pos = std::u32string::npos;
        if (newline_pos != std::u32string::npos ||
            space_pos != std::u32string::npos)
        {
            if (space_pos > newline_pos)
                end_pos = newline_pos;
            else
                end_pos = space_pos;
        }
        else
            end_pos = text.size();
        end_pos = fix_end_pos(text, pos, end_pos);
        http_pos.emplace_back((int)pos, (int)end_pos);
        pos = text.find(U"http://", pos + 1);
    }
    pos = text.find(U"https://", 0);
    while (pos != std::u32string::npos)
    {
        size_t newline_pos = text.find(U'\n', pos + 1);
        size_t space_pos = text.find(U' ', pos + 1);
        size_t end_pos = std::u32string::npos;
        if (newline_pos != std::u32string::npos ||
            space_pos != std::u32string::npos)
        {
            if (space_pos > newline_pos)
                end_pos = newline_pos;
            else
                end_pos = space_pos;
        }
        else
            end_pos = text.size();
        end_pos = fix_end_pos(text, pos, end_pos);
        http_pos.emplace_back((int)pos, (int)end_pos);
        pos = text.find(U"https://", pos + 1);
    }

    bool save_orig_string = (shape_flag & gui::SF_ENABLE_CLUSTER_TEST) != 0;
    if (!http_pos.empty())
        save_orig_string = true;

    int start = 0;
    std::shared_ptr<std::u32string> orig_string =
        std::make_shared<std::u32string>(text);
    for (unsigned l = 0; l < lines.size(); l++)
    {
        std::vector<ShapeGlyph> glyphs;
        if (l != 0)
        {
            gui::GlyphLayout gl = { 0 };
            gl.flags = gui::GLF_NEWLINE;
            gls.push_back(gl);
        }

        std::u32string& str = lines[l];
        if (str.empty())
        {
            start += 1;
            continue;
        }

        SBCodepointSequence codepoint_sequence;
        codepoint_sequence.stringEncoding = SBStringEncodingUTF32;
        codepoint_sequence.stringBuffer = (void*)str.c_str();
        codepoint_sequence.stringLength = str.size();

        // Extract the first bidirectional paragraph
        SBAlgorithmRef bidi_algorithm = SBAlgorithmCreate(&codepoint_sequence);
        SBParagraphRef first_paragraph = SBAlgorithmCreateParagraph(bidi_algorithm,
            0, (int32_t)-1, SBLevelDefaultLTR);
        SBUInteger paragraph_length = SBParagraphGetLength(first_paragraph);

        // Create a line consisting of whole paragraph and get its runs
        SBLineRef paragraph_line = SBParagraphCreateLine(first_paragraph, 0,
            paragraph_length);
        SBUInteger run_count = SBLineGetRunCount(paragraph_line);
        const SBRun* run_array = SBLineGetRunsPtr(paragraph_line);

        std::vector<bool> rtl_line, rtl_char;
        rtl_line.resize(str.size(), false);
        rtl_char.resize(str.size(), false);
        for (SBUInteger run = 0; run < run_count; run++)
        {
            FT_Face prev_face = NULL;
            std::vector<std::pair<FT_Face, SBInteger> > face_for_shape;
            SBInteger run_length = run_array[run].length;
            hb_buffer_flags_t hb_buffer_flags = static_cast<hb_buffer_flags_t>(
                HB_BUFFER_FLAG_BOT | HB_BUFFER_FLAG_EOT);
            hb_direction_t direction = run_array[run].level % 2 == 0 ?
                HB_DIRECTION_LTR : HB_DIRECTION_RTL;
            for (SBInteger l = 0; l < run_length; l++)
            {
                SBInteger i = l + run_array[run].offset;
                rtl_char[i] = direction == HB_DIRECTION_RTL;
                // Use the first character in line to determine paragraph
                // direction
                if (i == 0 && direction == HB_DIRECTION_RTL)
                    std::fill(rtl_line.begin(), rtl_line.end(), true);
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
                        // join marks), try to use the previous face so it is
                        // not hb_shape separately
                        cur_face = prev_face;
                        override_face = true;
                    }
                }
                FT_Face emoji_face = m_faces.size() > 1 ? m_faces[1] : NULL;
                if (m_has_color_emoji && !override_face &&
                    run_length > 1 &&
                    i < (SBInteger)(run_length + run_array[run].offset - 1) &&
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
                face_for_shape.emplace_back(cur_face, i);
            }
            if (face_for_shape.empty())
                continue;

            // Compare if different hb_font should be used in this run
            auto shape_compare = face_for_shape.front();
            auto it = face_for_shape.begin();

            // Depend on direction push front or back in this vector
            std::vector<ShapeGlyph> run_glyphs;
            while (it != face_for_shape.end())
            {
                const std::pair<FT_Face, SBInteger>& cur_compare = *it;
                if (cur_compare.first != shape_compare.first)
                {
                    hb_buffer_reset(m_hb_buffer);
                    int offset = shape_compare.second;
                    int length = cur_compare.second - offset;
                    FT_Face ft_face = shape_compare.first;
                    hb_buffer_add_utf32(m_hb_buffer,
                        (uint32_t*)(str.c_str() + offset), length, 0, -1);
                    hb_buffer_guess_segment_properties(m_hb_buffer);
                    hb_buffer_set_direction(m_hb_buffer, direction);
                    hb_buffer_set_flags(m_hb_buffer, hb_buffer_flags);
                    hb_shape(
                        m_hb_fonts[m_ft_faces_to_index[ft_face]],
                        m_hb_buffer, NULL, 0);
                    std::vector<ShapeGlyph> shaped_glyphs;
                    fill_shape_glyph(shaped_glyphs, m_hb_buffer, offset,
                        ft_face);
                    if (direction == HB_DIRECTION_LTR)
                    {
                        run_glyphs.insert(run_glyphs.end(),
                            shaped_glyphs.begin(), shaped_glyphs.end());
                    }
                    else
                    {
                        run_glyphs.insert(run_glyphs.begin(),
                            shaped_glyphs.begin(), shaped_glyphs.end());
                    }
                    shape_compare = cur_compare;
                    it = face_for_shape.erase(face_for_shape.begin(), it);
                    continue;
                }
                it++;
            }
            // Remaining pair if they use the same face
            if (!face_for_shape.empty())
            {
                hb_buffer_reset(m_hb_buffer);
                int offset = face_for_shape.front().second;
                int length = face_for_shape.back().second - offset + 1;
                FT_Face ft_face = face_for_shape.front().first;
                hb_buffer_add_utf32(m_hb_buffer,
                    (uint32_t*)(str.c_str() + offset), length, 0, -1);
                hb_buffer_guess_segment_properties(m_hb_buffer);
                hb_buffer_set_direction(m_hb_buffer, direction);
                hb_buffer_set_flags(m_hb_buffer, hb_buffer_flags);
                hb_shape(m_hb_fonts[m_ft_faces_to_index[ft_face]],
                    m_hb_buffer, NULL, 0);
                std::vector<ShapeGlyph> shaped_glyphs;
                fill_shape_glyph(shaped_glyphs, m_hb_buffer, offset, ft_face);
                if (direction == HB_DIRECTION_LTR)
                {
                    run_glyphs.insert(run_glyphs.end(),
                        shaped_glyphs.begin(), shaped_glyphs.end());
                }
                else
                {
                    run_glyphs.insert(run_glyphs.begin(),
                        shaped_glyphs.begin(), shaped_glyphs.end());
                }
            }
            glyphs.insert(glyphs.end(), run_glyphs.begin(), run_glyphs.end());
        }
        SBLineRelease(paragraph_line);
        SBParagraphRelease(first_paragraph);
        SBAlgorithmRelease(bidi_algorithm);

        if (!glyphs.empty())
        {
            std::vector<gui::GlyphLayout> cur_line;
            std::vector<bool> breakable;
            breakable.resize(str.size(), false);
            LineBreakingRules::insertBreakMark(str, breakable);
            translations->insertThaiBreakMark(str, breakable);
            for (unsigned idx = 0; idx < glyphs.size(); idx++)
            {
                // Skip some control characters
                if (str[glyphs[idx].cluster] == U'\t' ||
                    str[glyphs[idx].cluster] == U'\r')
                    continue;
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
                if (save_orig_string)
                    gl.orig_string = orig_string;
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
                // Add start offset to clusters
                for (int& each_cluster : gl.cluster)
                {
                    each_cluster += start;
                    for (auto& p : http_pos)
                    {
                        if (each_cluster >= p.first &&
                            each_cluster < p.second)
                            gl.flags |= gui::GLF_URL;
                    }
                }
            }
            gls.insert(gls.end(), cur_line.begin(), cur_line.end());
        }
        // Next str will have a newline
        start += str.size() + 1;
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
/** Convert text to glyph layouts for fast rendering with (optional) caching
 *  enabled.
 */
void FontManager::initGlyphLayouts(const core::stringw& text,
                                   std::vector<irr::gui::GlyphLayout>& gls,
                                   u32 shape_flag)
{
    if (GUIEngine::isNoGraphics() || text.empty())
        return;

    if ((shape_flag & gui::SF_DISABLE_CACHE) != 0)
    {
        shape(StringUtils::wideToUtf32(text), gls, shape_flag);
        return;
    }

    auto& cached_gls = getCachedLayouts(text);
    if (cached_gls.empty())
        shape(StringUtils::wideToUtf32(text), cached_gls, shape_flag);
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
    if (!GUIEngine::isNoGraphics())
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
    for (FT_Face face : normal_ttf)
    {
        if (!FT_HAS_COLOR(face) ||
            (FT_HAS_COLOR(face) && face->num_fixed_sizes == 0))
        {
            checkFTError(FT_Set_Pixel_Sizes(face, 0, m_shaping_dpi),
                "setting DPI");
        }
        m_hb_fonts.push_back(hb_ft_font_create(face, NULL));
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
        std::set<unsigned int> used_chars = translations->getCurrentAllChar();
        // First FontWithFace is RegularFace
        FaceTTF* ttf = m_fonts.front()->getFaceTTF();
        for (const unsigned int& c : used_chars)
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
