cmake_minimum_required(VERSION 3.30)

project(gui
    VERSION 0.1.0
    LANGUAGES C CXX
)

option(GUI_ENABLE_INSPECTOR "Enable the debug inspector UI" OFF)
option(GUI_ENABLE_INSTRUMENTATION "Enable render and layout instrumentation" ${GUI_ENABLE_INSPECTOR})
option(GUI_PROFILE "Build with profiling-friendly frame pointers" OFF)
set(METAL_CPP_ROOT "/Users/treja/metal-cpp" CACHE PATH "Path to metal-cpp")
set(METAL_CPP_EXTENSIONS_ROOT "/Users/treja/metal-cpp-extensions" CACHE PATH "Path to metal-cpp-extensions")

include(CTest)

add_subdirectory(vendor)
add_subdirectory(apple-extensions)
add_subdirectory(src)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
