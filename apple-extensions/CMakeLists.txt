if(NOT APPLE)
    return()
endif()

find_program(XCODEBUILD_EXECUTABLE xcodebuild REQUIRED)

set(APPLE_EXTENSIONS_OUTPUT_DIR "${PROJECT_BINARY_DIR}/apple-extensions")
set(APPKIT_EXTENSIONS_LIBRARY "${APPLE_EXTENSIONS_OUTPUT_DIR}/AppKit-Extensions")
set(MTK_EXTENSIONS_LIBRARY "${APPLE_EXTENSIONS_OUTPUT_DIR}/MTK-Extensions")

set(APPKIT_EXTENSIONS_LIBRARY "${APPKIT_EXTENSIONS_LIBRARY}" PARENT_SCOPE)
set(MTK_EXTENSIONS_LIBRARY "${MTK_EXTENSIONS_LIBRARY}" PARENT_SCOPE)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(APPLE_EXTENSIONS_CONFIGURATION Debug)
else()
    set(APPLE_EXTENSIONS_CONFIGURATION Release)
endif()

add_custom_command(
    OUTPUT "${APPKIT_EXTENSIONS_LIBRARY}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${APPLE_EXTENSIONS_OUTPUT_DIR}"
    COMMAND "${XCODEBUILD_EXECUTABLE}"
        -project "${CMAKE_CURRENT_SOURCE_DIR}/AppKit/AppKit-Extensions.xcodeproj"
        -scheme AppKit-Extensions
        -configuration "${APPLE_EXTENSIONS_CONFIGURATION}"
        -derivedDataPath "${PROJECT_BINARY_DIR}/xcode-derived/AppKit"
        "CONFIGURATION_BUILD_DIR=${APPLE_EXTENSIONS_OUTPUT_DIR}"
        CODE_SIGNING_ALLOWED=NO
        build
    DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/AppKit/Sources/AppKitExtensions.swift"
        "${CMAKE_CURRENT_SOURCE_DIR}/AppKit/AppKit-Extensions.xcodeproj/project.pbxproj"
    VERBATIM
)

add_custom_command(
    OUTPUT "${MTK_EXTENSIONS_LIBRARY}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${APPLE_EXTENSIONS_OUTPUT_DIR}"
    COMMAND "${XCODEBUILD_EXECUTABLE}"
        -project "${CMAKE_CURRENT_SOURCE_DIR}/MTK/MTK-Extensions.xcodeproj"
        -scheme MTK-Extensions-Library
        -configuration "${APPLE_EXTENSIONS_CONFIGURATION}"
        -derivedDataPath "${PROJECT_BINARY_DIR}/xcode-derived/MTK"
        "CONFIGURATION_BUILD_DIR=${APPLE_EXTENSIONS_OUTPUT_DIR}"
        CODE_SIGNING_ALLOWED=NO
        build
    DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/MTK/Sources/MTKTextures.swift"
        "${CMAKE_CURRENT_SOURCE_DIR}/MTK/MTK-Extensions.xcodeproj/project.pbxproj"
    VERBATIM
)

add_custom_target(gui_apple_extensions
    DEPENDS
        "${APPKIT_EXTENSIONS_LIBRARY}"
        "${MTK_EXTENSIONS_LIBRARY}"
)
