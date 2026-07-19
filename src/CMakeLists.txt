find_package(Freetype REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(PNG REQUIRED)
find_package(BZip2 REQUIRED)
find_package(ZLIB REQUIRED)

pkg_check_modules(HarfBuzz REQUIRED IMPORTED_TARGET harfbuzz)
pkg_check_modules(Resvg REQUIRED IMPORTED_TARGET resvg)

find_library(METAL_FRAMEWORK Metal REQUIRED)
find_library(METALKIT_FRAMEWORK MetalKit REQUIRED)
find_library(IMAGEIO_FRAMEWORK ImageIO REQUIRED)
find_library(FOUNDATION_FRAMEWORK Foundation REQUIRED)
find_library(QUARTZCORE_FRAMEWORK QuartzCore REQUIRED)
find_library(APPKIT_FRAMEWORK AppKit REQUIRED)
find_library(COCOA_FRAMEWORK Cocoa REQUIRED)

find_program(METAL_COMPILER
    NAMES metal
    HINTS "$ENV{MTL_TOOLS_PATH}"
)
find_program(METALLIB_LINKER
    NAMES metallib
    HINTS "$ENV{MTL_TOOLS_PATH}"
)

if(NOT METAL_COMPILER OR NOT METALLIB_LINKER)
    execute_process(
        COMMAND xcrun -sdk macosx --find metal
        OUTPUT_VARIABLE METAL_COMPILER
        OUTPUT_STRIP_TRAILING_WHITESPACE
        COMMAND_ERROR_IS_FATAL ANY
    )
    execute_process(
        COMMAND xcrun -sdk macosx --find metallib
        OUTPUT_VARIABLE METALLIB_LINKER
        OUTPUT_STRIP_TRAILING_WHITESPACE
        COMMAND_ERROR_IS_FATAL ANY
    )
endif()

set(GUI_SOURCES
    MTKTexture_loader.cpp
    bidi.cpp
    buffer_allocator.cpp
    color.cpp
    context_manager.cpp
    div.cpp
    element.cpp
    flex.cpp
    glyphCache.cpp
    glyphs.cpp
    grid.cpp
    image.cpp
    index.cpp
    instrumentation.cpp
    inspector.cpp
    main.cpp
    new_arch.cpp
    node_builder.cpp
    printers.cpp
    render_tree.cpp
    renderer.cpp
    sdf_helpers.cpp
    svg.cpp
    swift_object.cpp
    text.cpp
    textShaper.cpp
    text_bidi.cpp
    tree_manager.cpp
    window.cpp
)

set(METAL_SOURCES
    common.metal
    div_shaders.metal
    image_shaders.metal
    text_shaders.metal
)

set(METAL_AIR_FILES)
foreach(shader IN LISTS METAL_SOURCES)
    get_filename_component(shader_name "${shader}" NAME_WE)
    set(air_file "${PROJECT_BINARY_DIR}/shaders/${shader_name}.air")
    add_custom_command(
        OUTPUT "${air_file}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${PROJECT_BINARY_DIR}/shaders"
        COMMAND "${METAL_COMPILER}"
            -c "${CMAKE_CURRENT_SOURCE_DIR}/${shader}"
            -fmodules-cache-path="${PROJECT_BINARY_DIR}/module-cache"
            -o "${air_file}"
        DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/${shader}"
            "${CMAKE_CURRENT_SOURCE_DIR}/common.metal"
        VERBATIM
    )
    list(APPEND METAL_AIR_FILES "${air_file}")
endforeach()

set(GUI_METALLIB "${PROJECT_BINARY_DIR}/default.metallib")
add_custom_command(
    OUTPUT "${GUI_METALLIB}"
    COMMAND "${METALLIB_LINKER}" ${METAL_AIR_FILES} -o "${GUI_METALLIB}"
    DEPENDS ${METAL_AIR_FILES}
    VERBATIM
)
add_custom_target(gui_metallib DEPENDS "${GUI_METALLIB}")

add_executable(gui ${GUI_SOURCES})
add_dependencies(gui gui_apple_extensions gui_metallib)

target_compile_features(gui PRIVATE cxx_std_26)
set_target_properties(gui PROPERTIES
    CXX_EXTENSIONS NO
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}"
)

target_compile_definitions(gui PRIVATE
    GUI_ENABLE_INSPECTOR=$<BOOL:${GUI_ENABLE_INSPECTOR}>
    GUI_ENABLE_INSTRUMENTATION=$<BOOL:${GUI_ENABLE_INSTRUMENTATION}>
    _LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_NONE
)

target_compile_options(gui PRIVATE -fno-objc-arc)

if(GUI_PROFILE)
    target_compile_options(gui PRIVATE -fno-omit-frame-pointer)
endif()

target_include_directories(gui PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${METAL_CPP_ROOT}"
    "${METAL_CPP_EXTENSIONS_ROOT}"
)

target_link_libraries(gui PRIVATE
    gui::sheenbidi
    Freetype::Freetype
    PkgConfig::HarfBuzz
    PkgConfig::Resvg
    PNG::PNG
    BZip2::BZip2
    ZLIB::ZLIB
    "${APPKIT_EXTENSIONS_LIBRARY}"
    "${MTK_EXTENSIONS_LIBRARY}"
    "${METAL_FRAMEWORK}"
    "${METALKIT_FRAMEWORK}"
    "${IMAGEIO_FRAMEWORK}"
    "${FOUNDATION_FRAMEWORK}"
    "${QUARTZCORE_FRAMEWORK}"
    "${APPKIT_FRAMEWORK}"
    "${COCOA_FRAMEWORK}"
    swiftCore
    swiftMetal
    swiftMetalKit
    swiftFoundation
    swiftQuartzCore
    swiftAppKit
)

target_link_directories(gui PRIVATE
    /usr/lib/swift
    /opt/homebrew/opt/llvm/lib/c++
)

target_link_options(gui PRIVATE
    "LINKER:-rpath,/usr/lib/swift"
    "LINKER:-rpath,/opt/homebrew/opt/llvm/lib/c++"
)

add_custom_target(run
    COMMAND "$<TARGET_FILE:gui>"
    DEPENDS gui
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    USES_TERMINAL
)
