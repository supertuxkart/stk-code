add_definitions(-DSB_CONFIG_UNITY)
include_directories("Headers")
add_library(sheenbidi STATIC Source/SheenBidi.c)
