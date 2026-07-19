add_library(gui_sheenbidi STATIC
    SheenBidi/Source/SheenBidi.c
)

add_library(gui::sheenbidi ALIAS gui_sheenbidi)

target_compile_features(gui_sheenbidi PRIVATE c_std_11)

target_compile_definitions(gui_sheenbidi PRIVATE
    SB_CONFIG_UNITY
)

target_include_directories(gui_sheenbidi
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/SheenBidi/Headers"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/SheenBidi/Source"
)
