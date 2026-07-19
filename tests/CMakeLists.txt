add_executable(gui_bidi_test
    bidi.cpp
    "${PROJECT_SOURCE_DIR}/src/bidi.cpp"
    "${PROJECT_SOURCE_DIR}/src/text_bidi.cpp"
)

target_compile_features(gui_bidi_test PRIVATE cxx_std_26)

set_target_properties(gui_bidi_test PROPERTIES
    CXX_EXTENSIONS NO
)

target_include_directories(gui_bidi_test PRIVATE
    "${PROJECT_SOURCE_DIR}/src"
)

target_link_libraries(gui_bidi_test PRIVATE
    gui::sheenbidi
)

add_test(NAME gui.bidi COMMAND gui_bidi_test)
