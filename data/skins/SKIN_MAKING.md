# Making a skin

To make your own skin, the easiest way is to start with the files from a standard skin
and modifying them as needed.

There are two types of images : some will be simply stretched as a whole, others will
have non-stretchable borders (you cannot choose which one you must use, it's hardcoded
for each element type; though, as you will see below, for all "advanced stretching" images
you can easily fake "simple stretch")

All elements will have at least 2 properties :
        type="X"                                sets what you're skinning with this entry
        image="skinDirectory/imageName.png"     sets which image is used for this element

Most elements also support states :
        state="neutral"
        state="focused"
        state="down"
You can thus give different looks for different states.  Not all widgets support all states,
see entries and comments below to know what's supported.
Note that checkboxes are an exception and have the following styles :
    "neutral+unchecked"
    "neutral+checked"
    "focused+unchecked"
    "focused+checked"
    "deactivated+unchecked"
    "deactivated+checked"

"Advanced stretching" images are split this way :

     +----+--------------------+----+
     |    |                    |    |
     +----+--------------------+----+
     |    |                    |    |
     |    |                    |    |     
     |    |                    |    |     
     +----+--------------------+----+
     |    |                    |    | 
     +----+--------------------+----+
     
The center border will be stretched in all directions. The 4 corners will not stretch at all.
Horizontal borders will stretch horizontally, verticallt borders will stretch vertically.
Use properties left_border="X" right_border="X" top_border="X" bottom_border="X" to specify
the size of each border in pixels (setting all borders to '0' makes the whole image scaled).

In some cases, you may not want vertical stretching to occur (like if the left and right sides
of the image must not be stretched vertically, e.g. for the spinner). In this case, pass
parameter preserve_h_aspect_ratios="true" to make the left and right areas stretch by keeping
their aspect ratio.

Some components may fill the full inner area with stuff; others will only take a smaller
area at the center. To adjust for this, there are properties "hborder_out_portion" and "vborder_out_portion"
that take a float from 0 to 1, representing the percentage of each border that goes out of the widget's
area (this might include stuff like shadows, etc.). The 'h' one is for horizontal borders,
the 'v' one is for vertical borders.

Finnally : the image is split, as shown above, into 9 areas. In some cases, you may not want
all areas to be rendered. Then you can pass parameter areas="body+left+right+top+bottom"
and explicitely specify which parts you want to see. The 4 corner areas are only visible
when the border that intersect at this corner are enabled.

When there is a common="y" with image tag, the image will be loaded only from data/skins/common in stk-code.

Any information not specified in the stkskin.xml file of a theme will be inherited from the specified
base theme, if any. To specify a base theme, add base_theme="themename" to the `<skin>` tag.

To use an icon theme, place the replacement icons (PNG or SVG) into [skin folder]/data/gui/icons
STK will prefer these icons first, if not found it will fallback to icons from the base theme(s).

For TTF specify the list like the following, for normal and digit ttf it will be added at the beginning of the
font list in STK, so those TTF will be used first, and any missing characters will be rendered from the base
theme font list. For color emoji ttf it will replace the base theme color emoji directly. You are not required
to specify all types of ttf.

```
<advanced normal_ttf="xxx.ttf yyy.ttf"
          digit_ttf="zzz.ttf"
          color_emoji_ttf="www.ttf"/>
```