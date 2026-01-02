#!/bin/bash
set -euo pipefail

usage() {
    echo "Usage: $0 [option]"
    echo "Options:"
    echo "  build        Build the project"
    echo "  run          Run the built executable"
    echo "  buildrun     Build and run the executable"
    echo "  debug        Build and run with lldb"
    exit 1
}

if [ $# -eq 0 ]; then
    usage
fi

case "$1" in
    build)
        /Applications/Xcode.app/Contents/Developer/usr/bin/xcodebuild -project gui.xcodeproj -scheme "gui" -configuration Debug -sdk macosx -derivedDataPath ./build clean build -quiet
        ;;
    run)
        ./build/Build/Products/Debug/gui
        ;;
    buildrun)
        /Applications/Xcode.app/Contents/Developer/usr/bin/xcodebuild -project gui.xcodeproj -scheme "gui" -configuration Debug -sdk macosx -derivedDataPath ./build clean build -quiet
        ./build/Build/Products/Debug/gui
        ;;
    debug)
        /Applications/Xcode.app/Contents/Developer/usr/bin/xcodebuild -project gui.xcodeproj -scheme "gui" -configuration Debug -sdk macosx -derivedDataPath ./build clean build
        lldb ./build/Build/Products/Debug/gui -o run
        ;;
    *)
        usage
        ;;
esac


# #!/bin/bash
# set -euo pipefail

# # Configuration
# BINARY_NAME="gui"
# BIN_OUT="./bin"

# # Paths from your environment
# SDK_PATH="/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk"
# METAL_CPP="/Users/treja/metal-cpp"
# METAL_CPP_EXT="/Users/treja/metal-cpp-extensions"
# FREETYPE_INC="/opt/homebrew/opt/freetype/include/freetype2"

# # Static Library Paths
# LIB_FREETYPE="/opt/homebrew/Cellar/freetype/2.13.3/lib/libfreetype.a"
# LIB_MTK_EXT="/Users/treja/projects/gui/MTK-Extensions-Library/MTK-Extensions"
# LIB_APPKIT_EXT="/Users/treja/projects/gui/AppKit-Extensions-Library/AppKit-Extensions"

# LLVM_DIR="/opt/homebrew/opt/llvm"

# usage() {
#     echo "Usage: $0 [option]"
#     echo "Options:"
#     echo "  build        Compile C++ and link with static libs"
#     echo "  run          Run the binary"
#     echo "  buildrun     Build and run"
#     echo "  debug        Build and debug with lldb"
#     exit 1
# }

# build_shaders() {
#     echo "Compiling Metal shaders..."

#     DEVELOPER_DIR="/Applications/Xcode.app/Contents/Developer"
#     export DEVELOPER_DIR

#     # locate tools without running them
#     METAL="$(xcrun -sdk macosx --find metal 2>/dev/null || command -v metal || true)"
#     METALLIB="$(xcrun -sdk macosx --find metallib 2>/dev/null || command -v metallib || true)"

#     if [ -z "$METAL" ] || [ -z "$METALLIB" ]; then
#         echo "ERROR: could not locate metal/metallib." >&2
#         echo "METAL='$METAL' METALLIB='$METALLIB'"
#         return 1
#     fi

#     echo "PWD: $(pwd)"
#     echo "Checking gui/*.metal …"

#     shopt -s nullglob
#     METAL_FILES=(gui/*.metal)
#     shopt -u nullglob

#     echo "METAL_FILES count: ${#METAL_FILES[@]}"
#     for f in "${METAL_FILES[@]}"; do
#         echo "  $f"
#     done
#     if [ ${#METAL_FILES[@]} -eq 0 ]; then
#         echo "ERROR: No .metal files found in gui/"
#         return 1
#     fi

#     BIN_OUT="${BIN_OUT:-build/bin}"
#     mkdir -p "$BIN_OUT"

#     # Compile each .metal -> .air separately (one output per input)
#     AIR_FILES=()
#     echo "Compiling each .metal -> .air"
#     for m in "${METAL_FILES[@]}"; do
#         out_air="$BIN_OUT/$(basename "$m" .metal).air"
#         echo "  $m -> $out_air"
#         if ! "$METAL" -c "$m" -o "$out_air"; then
#             echo "ERROR: metal failed for $m" >&2
#             # keep the .air for debugging if desired; comment out the next line to preserve it
#             # rm -f "$out_air"
#             return 1
#         fi
#         AIR_FILES+=( "$out_air" )
#     done

#     # Link all .air files into one metallib
#     echo "Linking ${#AIR_FILES[@]} .air files -> $BIN_OUT/default.metallib"
#     if ! "$METALLIB" "${AIR_FILES[@]}" -o "$BIN_OUT/default.metallib"; then
#         echo "ERROR: metallib failed." >&2
#         return 1
#     fi

#     # cleanup individual .air files (optional)
#     for a in "${AIR_FILES[@]}"; do rm -f "$a"; done

#     echo "Shaders compiled successfully: $BIN_OUT/default.metallib"
#     return 0
# }

# # build_cpp() {
# #     mkdir -p "$BIN_OUT"
# #     OBJ_DIR="$BIN_OUT/obj"
# #     mkdir -p "$OBJ_DIR"

# #     echo "Compiling with brew clang++ (objects -> $OBJ_DIR)..."

# #     OBJS=""
# #     for src in gui/*.cpp; do
# #         if [ ! -f "$src" ]; then
# #             echo "No sources found in gui/*.cpp"; break
# #         fi
# #         base="$(basename "$src" .cpp)"
# #         obj="$OBJ_DIR/${base}.o"
# #         echo "  $src -> $obj"
# #         clang++ -std=c++23 -isysroot "$SDK_PATH" \
# #             -fno-objc-arc \
# #             -Igui -I"$METAL_CPP" -I"$METAL_CPP_EXT" -I"$FREETYPE_INC" \
# #             -stdlib=libc++ -c "$src" -o "$obj"
# #         OBJS="$OBJS $obj"
# #     done

# #     echo "Linking..."

# #     # # Link order: objects -> your static libs -> dependency libs -> frameworks -> swift core runtimes
# #     # clang++ -std=c++23 -isysroot "$SDK_PATH" \
# #     #     -fno-objc-arc \
# #     #     -stdlib=libc++ \
# #     #     $OBJS \
# #     #     "$LIB_MTK_EXT" "$LIB_APPKIT_EXT" "$LIB_FREETYPE" \
# #     #     -L/opt/homebrew/lib -lpng -lbz2 -lz \
# #     #     -framework Metal -framework MetalKit -framework Foundation -framework QuartzCore -framework AppKit -framework Cocoa \
# #     #     -L/usr/lib/swift -lswiftCore -lswiftMetal -lswiftMetalKit -lswiftFoundation -lswiftQuartzCore -lswiftAppKit \
# #     #     -Wl,-rpath,/usr/lib/swift  \
# #     #     -lc++ -lc++abi \
# #     #     -o "$BIN_OUT/$BINARY_NAME"
# #     LLVM_C_PP_PATH="/opt/homebrew/opt/llvm/lib/c++"

# #     clang++ -std=c++23 -isysroot "$SDK_PATH" \
# #         -D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_NONE \
# #         -fno-objc-arc \
# #         -Igui -I"$METAL_CPP" -I"$METAL_CPP_EXT" -I"$FREETYPE_INC" \
# #         -stdlib=libc++ -c "$src" -o "$obj"

# #     if [ $? -ne 0 ]; then
# #         echo "Link failed."
# #         return 1
# #     fi

# #     echo "Build successful: $BIN_OUT/$BINARY_NAME"
# # }


# build_cpp() {
#     mkdir -p "$BIN_OUT"
#     OBJ_DIR="$BIN_OUT/obj"
#     mkdir -p "$OBJ_DIR"

#     # Locate system C++ headers within your SDK
#     SYS_CXX_INC="$SDK_PATH/usr/include/c++/v1"

#     echo "Compiling with clang++ (Strict System Headers)..."

#     OBJS=""
#     for src in gui/*.cpp; do
#         if [ ! -f "$src" ]; then continue; fi
#         base="$(basename "$src" .cpp)"
#         obj="$OBJ_DIR/${base}.o"
        
#         # 1. -nostdinc++: Ignore Homebrew's libc++ headers
#         # 2. -isystem: Use the SDK's libc++ headers instead
#         # 3. Hardening: Keep your required flag
#         clang++ -std=c++23 -isysroot "$SDK_PATH" \
#             -nostdinc++ \
#             -isystem "$SYS_CXX_INC" \
#             -D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_NONE \
#             -fno-objc-arc \
#             -Igui -I"$METAL_CPP" -I"$METAL_CPP_EXT" -I"$FREETYPE_INC" \
#             -stdlib=libc++ -c "$src" -o "$obj"
        
#         if [ $? -ne 0 ]; then return 1; fi
#         OBJS="$OBJS $obj"
#     done

#     echo "Linking..."

#     # Linker step remains the "fast way forward"
#     clang++ -std=c++23 -isysroot "$SDK_PATH" \
#         -L"$SDK_PATH/usr/lib" \
#         -D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_NONE \
#         -fno-objc-arc \
#         -stdlib=libc++ \
#         $OBJS \
#         "$LIB_MTK_EXT" "$LIB_APPKIT_EXT" "$LIB_FREETYPE" \
#         -L/opt/homebrew/lib -lpng -lbz2 -lz \
#         -framework Metal -framework MetalKit -framework Foundation -framework QuartzCore -framework AppKit -framework Cocoa \
#         -L/usr/lib/swift -lswiftCore -lswiftMetal -lswiftMetalKit -lswiftFoundation -lswiftQuartzCore -lswiftAppKit \
#         -Wl,-rpath,/usr/lib/swift \
#         -lc++ -lc++abi \
#         -o "$BIN_OUT/$BINARY_NAME"

#     if [ $? -ne 0 ]; then
#         echo "Link failed."
#         return 1
#     fi

#     echo "Build successful: $BIN_OUT/$BINARY_NAME"
# }

# if [ $# -eq 0 ]; then
#     usage
# fi

# case "$1" in
#     build)
#         build_shaders
#         build_cpp
#         ;;
#     run)
#         "$BIN_OUT/$BINARY_NAME"
#         ;;
#     buildrun)
#         build_shaders
#         build_cpp
#         "$BIN_OUT/$BINARY_NAME"
#         ;;
#     debug)
#         build_shaders
#         build_cpp
#         lldb "$BIN_OUT/$BINARY_NAME" -o run
#         ;;
#     *)
#         usage
#         ;;
# esac