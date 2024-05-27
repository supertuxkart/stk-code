# This color variation was made using the following commands:

for i in *.png ; do convert "$i" -colorspace gray -fill "#ff0000" -tint 100 "$i" ; done
for i in *focus.png ; do convert "$i" -colorspace gray -fill "#990000" -tint 100 "$i" ; done
for i in *focused.png ; do convert "$i" -colorspace gray -fill "#990000" -tint 100 "$i" ; done
convert bottom_bar.png -colorspace gray -fill "#690000" -tint 100 bottom_bar.png

# table_header_down manually re-colored
# gauge_fill and spinner_fill were manually colored to #ffa6b3
# achievement and friend were manually re-created from the source SVG with the background color changed.