if (MSVC)
    # warning level 4 and all warnings as errors
    add_compile_options(/W4 /WX)
else()
    # lots of warnings # and evenutally all warnings as errors -Werror
    add_compile_options(-g -Wall -Wextra -pedantic)
endif()
