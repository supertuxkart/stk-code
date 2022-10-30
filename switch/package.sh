nacptool --create "SuperTuxKart" "Many People" "${PROJECT_VERSION}" control.nacp
elf2nro bin/supertuxkart.elf bin/stk.nro --nacp=control.nacp --icon=../switch/supertuxkart_256.jpg
