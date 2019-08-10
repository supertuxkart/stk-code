// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __GLYPH_LAYOUT_H_INCLUDED__
#define __GLYPH_LAYOUT_H_INCLUDED__

#include "irrTypes.h"
#include "dimension2d.h"

#include <algorithm>
#include <numeric>
#include <vector>

namespace irr
{
namespace gui
{

enum GlyphLayoutFlag
{
GLF_RTL_LINE = 1, /* This line from this glyph is RTL. */
GLF_RTL_CHAR = 2, /* This character(s) from this glyph is RTL. */
GLF_BREAKABLE = 4, /* This glyph is breakable when line breaking. */
GLF_QUICK_DRAW = 8, /* This glyph is not created by libraqm, which get x_advance_x directly from font. */
GLF_NEWLINE = 16, /* This glyph will start a newline. */
GLF_COLORED = 32 /* This glyph is a colored one (for example emoji). */
};

enum GlyphLayoutDraw
{
GLD_NONE = 0, /* Default flag. */
GLD_MARKED = 1, /* This glyph will be drawn with background marked for marked text. */
GLD_COMPOSING = 2 /* This glyph will be drawn with underline (for example composing text). */
};

//! GlyphLayout copied from libraqm.
struct GlyphLayout
{
u32 index;
s32 x_advance;
//s32 y_advance; we don't use y_advance atm
s32 x_offset;
s32 y_offset;
/* Above variable is same for raqm_glyph_t */
// If some characters share the same glyph
std::vector<s32> cluster;
std::vector<u8> draw_flags;
//! used to sorting back the visual order after line breaking
u32 original_index;
u16 flags;
//! this is the face_idx used in stk face ttf
u16 face_idx;
};

namespace Private
{
    inline void breakLine(std::vector<GlyphLayout> gls, f32 max_line_width,
        f32 inverse_shaping, f32 scale, std::vector<std::vector<GlyphLayout> >& result);
}

inline void eraseTopLargerThan(std::vector<GlyphLayout>& gls,
    f32 height_per_line, f32 max_height)
{
    if (max_height < height_per_line * 2.0f)
        return;

    std::vector<u32> newline_positions;
    for (unsigned i = 0; i < gls.size(); i++)
    {
        const GlyphLayout& glyph = gls[i];
        if ((glyph.flags & GLF_NEWLINE) != 0)
        {
            newline_positions.push_back(i);
            continue;
        }
    }

    // Handle the first line
    f32 total_height = height_per_line +
        (f32)newline_positions.size() * height_per_line;
    if (total_height > max_height)
    {
        u32 idx = (u32)((total_height - max_height) / height_per_line);
        if (idx < newline_positions.size())
        {
            auto end_it = gls.begin() + newline_positions[idx] + 1;
            if (end_it != gls.end())
                gls.erase(gls.begin(), end_it);
        }
    }
}

inline core::dimension2d<u32> getGlyphLayoutsDimension(
    const std::vector<GlyphLayout>& gls, s32 height_per_line, f32 inverse_shaping,
    f32 scale, s32 cluster = -1)
{
    core::dimension2d<f32> dim(0.0f, 0.0f);
    core::dimension2d<f32> this_line(0.0f, (f32)height_per_line);

    for (unsigned i = 0; i < gls.size(); i++)
    {
        const GlyphLayout& glyph = gls[i];
        if ((glyph.flags & GLF_NEWLINE) != 0)
        {
            dim.Height += this_line.Height;
            if (dim.Width < this_line.Width)
                dim.Width = this_line.Width;
            this_line.Width = 0;
            continue;
        }
        f32 cur_width = (s32)(glyph.x_advance * inverse_shaping) * scale;
        bool found_cluster = false;
        // Cursor positioning
        if (cluster != -1)
        {
            auto it = std::find(glyph.cluster.begin(), glyph.cluster.end(), cluster);
            if (it != glyph.cluster.end() &&
                (i == gls.size() - 1 || cluster != gls[i + 1].cluster.front()))
            {
                found_cluster = true;
                // Get cluster ratio to total glyph width, so for example
                // cluster 0 in ffi glyph will be 0.333
                f32 ratio = f32(it - glyph.cluster.begin() + 1) /
                    (f32)glyph.cluster.size();
                // Show cursor in left side, so no need to add width
                if ((glyph.flags & GLF_RTL_CHAR) != 0)
                    cur_width = 0;
                else
                    cur_width *= ratio;
            }
        }
        this_line.Width += cur_width;
        if (found_cluster)
            break;
    }

    dim.Height += this_line.Height;
    if (dim.Width < this_line.Width)
        dim.Width = this_line.Width;

    core::dimension2d<u32> ret_dim(0, 0);
    ret_dim.Width = (u32)(dim.Width + 0.9f); // round up
    ret_dim.Height = (u32)(dim.Height + 0.9f);

    return ret_dim;
}

inline s32 getCurosrFromDimension(f32 x, f32 y,
    const std::vector<GlyphLayout>& gls, s32 height_per_line,
    f32 inverse_shaping, f32 scale)
{
    if (gls.empty())
        return 0;
    f32 total_width = 0.0f;
    for (unsigned i = 0; i < gls.size(); i++)
    {
        const GlyphLayout& glyph = gls[i];
        if ((glyph.flags & GLF_NEWLINE) != 0)
        {
            // TODO: handling newline
            break;
        }
        f32 cur_width = (s32)(glyph.x_advance * inverse_shaping) * scale;
        if (glyph.cluster.size() == 1)
        {
            // One more character threshold because we show the cursor position
            // opposite side for RTL character
            if (glyph.flags & GLF_RTL_CHAR)
            {
                if (i == 0 && cur_width * 0.5 > x)
                    return glyph.cluster.front() + 1;
                if (total_width + cur_width * 1.5 > x)
                    return glyph.cluster.front();
            }
            else
            {
                if (i == 0 && cur_width * 0.5 > x)
                    return 0;
                if (total_width + cur_width * 0.5 > x)
                    return glyph.cluster.front();
            }
        }
        else if (total_width + cur_width > x)
        {
            // Handle glyph like 'ffi'
            f32 each_cluster_width = cur_width / (f32)glyph.cluster.size();
            f32 remain_width = x - total_width;
            total_width = 0.0f;
            for (unsigned j = 0; j < glyph.cluster.size(); j++)
            {
                if (total_width + each_cluster_width * 0.5 > remain_width)
                    return glyph.cluster[j];
                total_width += each_cluster_width;
            }
            return glyph.cluster.back();
        }
        total_width += cur_width;
    }
    return gls.back().flags & GLF_RTL_CHAR ?
        0 : gls.back().cluster.back() + 1;
}

inline std::vector<f32> getGlyphLayoutsWidthPerLine(
    const std::vector<GlyphLayout>& gls, f32 inverse_shaping, f32 scale)
{
    std::vector<f32> result;
    f32 cur_width = 0.0f;
    for (auto& glyph : gls)
    {
        if ((glyph.flags & GLF_NEWLINE) != 0)
        {
            result.push_back(cur_width);
            cur_width = 0;
            continue;
        }
        cur_width += (s32)(glyph.x_advance * inverse_shaping) * scale;
    }

    result.push_back(cur_width);
    return result;
}

inline void breakGlyphLayouts(std::vector<GlyphLayout>& gls, f32 max_line_width,
                              f32 inverse_shaping, f32 scale)
{
    if (gls.size() < 2)
        return;
    std::vector<std::vector<GlyphLayout> > broken_line;
    u32 start = 0;
    for (u32 i = 0; i < gls.size(); i++)
    {
        GlyphLayout& glyph = gls[i];
        if ((glyph.flags & GLF_NEWLINE) != 0)
        {
            Private::breakLine({ gls.begin() + start, gls.begin() + i},
                max_line_width, inverse_shaping, scale, broken_line);
            start = i + 1;
        }
    }
    if (start - gls.size() - 1 > 0)
    {
        Private::breakLine({ gls.begin() + start, gls.begin() + gls.size() },
            max_line_width, inverse_shaping, scale, broken_line);
    }

    gls.clear();
    // Sort glyphs in original order
    for (u32 i = 0; i < broken_line.size(); i++)
    {
        if (i != 0)
        {
            gui::GlyphLayout gl = { 0 };
            gl.flags = gui::GLF_NEWLINE;
            gls.push_back(gl);
        }
        auto& line = broken_line[i];
        std::sort(line.begin(), line.end(), []
            (const irr::gui::GlyphLayout& a_gi,
            const irr::gui::GlyphLayout& b_gi)
            {
                return a_gi.original_index < b_gi.original_index;
            });
        for (auto& glyph : line)
            gls.push_back(glyph);
    }
}

namespace Private
{
    /** Used it only for single line (ie without line breaking mark). */
    inline f32 getGlyphLayoutsWidth(const std::vector<GlyphLayout>& gls,
                                    f32 inverse_shaping, f32 scale)
    {
        return std::accumulate(gls.begin(), gls.end(), 0.0f,
            [inverse_shaping, scale] (const f32 previous,
                const irr::gui::GlyphLayout& cur_gi)
            {
                return previous + (s32)(cur_gi.x_advance * inverse_shaping) * scale;
            });
    }

    inline void breakLine(std::vector<GlyphLayout> gls, f32 max_line_width,
        f32 inverse_shaping, f32 scale,
        std::vector<std::vector<GlyphLayout> >& result)
    {
        const f32 line_size = getGlyphLayoutsWidth(gls, inverse_shaping, scale);
        if (line_size <= max_line_width)
        {
            result.emplace_back(std::move(gls));
            return;
        }

        // Sort glyphs in logical order
        std::sort(gls.begin(), gls.end(), []
            (const GlyphLayout& a_gi, const GlyphLayout& b_gi)
            {
                return a_gi.cluster.front() < b_gi.cluster.front();
            });

        u32 end = 0;
        s32 start = 0;
        f32 total_width = 0.0f;

        for (; end < gls.size(); end++)
        {
            f32 cur_width = (s32)(gls[end].x_advance * inverse_shaping) * scale;
            if (cur_width > max_line_width)
            {
                // Very large glyph
                result.push_back({gls[end]});
                start = end;
            }
            else if (cur_width + total_width <= max_line_width)
            {
                total_width += cur_width;
            }
            else
            {
                int break_point = end - 1;
                do
                {
                    if (break_point <= 0 || break_point == start)
                    {
                        // Forcely break at line ending position if no break
                        // mark, this fix text without space of out network
                        // lobby
                        result.push_back(
                            {gls.begin() + start, gls.begin() + end});
                        start = end;
                        total_width = (s32)(gls[end].x_advance * inverse_shaping) * scale;
                        break;
                    }
                    if ((gls[break_point].flags & GLF_BREAKABLE) != 0)
                    {
                        result.push_back(
                            {gls.begin() + start, gls.begin() + break_point + 1});
                        end = start = break_point + 1;
                        total_width = (s32)(gls[end].x_advance * inverse_shaping) * scale;
                        break;
                    }
                    break_point--;
                }
                while (break_point >= start);
            }
        }
        if (gls.begin() + start != gls.end())
        {
            result.push_back({gls.begin() + start, gls.end()});
        }
    }
}

} // end namespace gui
} // end namespace irr

#endif

