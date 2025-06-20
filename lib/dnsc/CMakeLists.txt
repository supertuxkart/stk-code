add_library(dnsc STATIC dns.c)

set_target_properties(dnsc PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF
)
