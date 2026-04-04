# #!/bin/bash
# set -euo pipefail

# usage() {
#     echo "Usage: $0 [option]"
#     echo "Options:"
#     echo "  build        Build the project"
#     echo "  run          Run the built executable"
#     echo "  buildrun     Build and run the executable"
#     echo "  debug        Build and run with lldb"
#     exit 1
# }

# if [ $# -eq 0 ]; then
#     usage
# fi

# case "$1" in
#     build)
#         /Applications/Xcode.app/Contents/Developer/usr/bin/xcodebuild -project gui.xcodeproj -scheme "gui" -configuration Debug -sdk macosx -derivedDataPath ./build clean build -quiet
#         ;;
#     run)
#         ./build/Build/Products/Debug/gui
#         ;;
#     buildrun)
#         /Applications/Xcode.app/Contents/Developer/usr/bin/xcodebuild -project gui.xcodeproj -scheme "gui" -configuration Debug -sdk macosx -derivedDataPath ./build clean build -quiet
#         ./build/Build/Products/Debug/gui
#         ;;
#     debug)
#         /Applications/Xcode.app/Contents/Developer/usr/bin/xcodebuild -project gui.xcodeproj -scheme "gui" -configuration Debug -sdk macosx -derivedDataPath ./build clean build
#         lldb ./build/Build/Products/Debug/gui -o run
#         ;;
#     *)
#         usage
#         ;;
# esac


#!/bin/bash
set -euo pipefail

BINARY_NAME="gui"
BIN_OUT="./bin"
OBJ_DIR="$BIN_OUT/obj"

SDK_PATH="/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk"
METAL_CPP="/Users/treja/metal-cpp"
METAL_CPP_EXT="/Users/treja/metal-cpp-extensions"
FREETYPE_INC="/opt/homebrew/opt/freetype/include/freetype2"

LIB_FREETYPE="/opt/homebrew/Cellar/freetype/2.13.3/lib/libfreetype.a"
LIB_MTK_EXT="/Users/treja/projects/gui/MTK-Extensions-Library/MTK-Extensions"
LIB_APPKIT_EXT="/Users/treja/projects/gui/AppKit-Extensions-Library/AppKit-Extensions"

CXXFLAGS=(
    -std=c++23
    -isysroot "$SDK_PATH"
    -fno-objc-arc
    -D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_NONE
    -stdlib=libc++
    -Igui
    -I"$METAL_CPP"
    -I"$METAL_CPP_EXT"
    -I"$FREETYPE_INC"
    -O0 -g  # debug build
)

usage() {
    echo "Usage: $0 [build|run|buildrun|debug]"
    exit 1
}

needs_rebuild() {
    local src="$1" obj="$2" dep="$3"
    # Rebuild if obj missing or src is newer
    [[ ! -f "$obj" ]] || [[ "$src" -nt "$obj" ]] && return 0
    # Check header dependencies from .d file
    [[ -f "$dep" ]] || return 0
    while IFS= read -r line; do
        # Strip trailing backslash continuations and leading whitespace
        line="${line%\\}"
        line="${line#"${line%%[![:space:]]*}"}"
        # Skip the target: prefix on the first line
        line="${line#*: }"
        for hdr in $line; do
            [[ -f "$hdr" && "$hdr" -nt "$obj" ]] && return 0
        done
    done < "$dep"
    return 1
}

build_shaders() {
    echo "Compiling Metal shaders..."
    local METAL METALLIB
    METAL="$(xcrun -sdk macosx --find metal)"
    METALLIB="$(xcrun -sdk macosx --find metallib)"

    mkdir -p "$BIN_OUT"
    local AIR_FILES=()

    for m in gui/*.metal; do
        [[ -f "$m" ]] || { echo "No .metal files found"; return 1; }
        local air="$BIN_OUT/$(basename "$m" .metal).air"
        if needs_rebuild "$m" "$air" ""; then
            echo "  shader: $m"
            "$METAL" -c "$m" -o "$air"
        fi
        AIR_FILES+=("$air")
    done

    local metallib="$BIN_OUT/default.metallib"
    # Relink metallib if any .air is newer than the metallib
    local relink=0
    for air in "${AIR_FILES[@]}"; do
        [[ "$air" -nt "$metallib" ]] && relink=1 && break
    done
    if [[ $relink -eq 1 || ! -f "$metallib" ]]; then
        echo "  linking metallib..."
        "$METALLIB" "${AIR_FILES[@]}" -o "$metallib"
    fi
    echo "Shaders done."
}

build_cpp() {
    mkdir -p "$OBJ_DIR"

    local LLVM_PREFIX
    LLVM_PREFIX="$(brew --prefix llvm)"

    local OBJS=()
    local SRCS_TO_COMPILE=()
    local any_rebuilt=0

    for src in gui/*.cpp; do
        [[ -f "$src" ]] || continue
        local base obj dep
        base="$(basename "$src" .cpp)"
        obj="$OBJ_DIR/${base}.o"
        dep="$OBJ_DIR/${base}.d"
        OBJS+=("$obj")

        if needs_rebuild "$src" "$obj" "$dep"; then
            SRCS_TO_COMPILE+=("$src")
            any_rebuilt=1
        fi
    done

    if [[ ${#SRCS_TO_COMPILE[@]} -gt 0 ]]; then
        local NPROC
        NPROC="$(sysctl -n hw.logicalcpu)"
        echo "Compiling ${#SRCS_TO_COMPILE[@]} file(s) with $NPROC jobs..."

        local fail_marker="$OBJ_DIR/.compile_failed"
        rm -f "$fail_marker"

        printf '%s\0' "${SRCS_TO_COMPILE[@]}" | xargs -0 -P "$NPROC" -I{} bash -c '
            src="$1"
            base="$(basename "$src" .cpp)"
            obj="'"$OBJ_DIR"'/${base}.o"
            dep="'"$OBJ_DIR"'/${base}.d"
            echo "  compile: $src"
            if ! "'"$LLVM_PREFIX"'/bin/clang++" '"$(printf " %q" "${CXXFLAGS[@]}")"' \
                -I"'"$LLVM_PREFIX"'/include/c++/v1" \
                -MMD -MF "$dep" \
                -c "$src" -o "$obj"; then
                echo "Error: compile failed for $src"
                touch "'"$fail_marker"'"
            fi
        ' _ {}

        if [[ -f "$fail_marker" ]]; then
            rm -f "$fail_marker"
            return 1
        fi
    fi

    local bin="$BIN_OUT/$BINARY_NAME"
    if [[ $any_rebuilt -eq 1 || ! -f "$bin" ]]; then
        echo "Linking $BINARY_NAME..."
        if ! "$LLVM_PREFIX/bin/clang++" \
            -std=c++23 \
            -isysroot "$SDK_PATH" \
            -fno-objc-arc \
            -stdlib=libc++ \
            "${OBJS[@]}" \
            "$LIB_MTK_EXT" "$LIB_APPKIT_EXT" "$LIB_FREETYPE" \
            -L"$LLVM_PREFIX/lib/c++" \
            -Wl,-rpath,"$LLVM_PREFIX/lib/c++" \
            -L/opt/homebrew/lib -lpng -lbz2 -lz \
            -framework Metal -framework MetalKit -framework Foundation \
            -framework QuartzCore -framework AppKit -framework Cocoa \
            -L/usr/lib/swift \
            -lswiftCore -lswiftMetal -lswiftMetalKit \
            -lswiftFoundation -lswiftQuartzCore -lswiftAppKit \
            -Wl,-rpath,/usr/lib/swift \
            -lc++ \
            -o "$bin"; then
            echo "Error: link failed"
            return 1
        fi
        echo "Build successful: $bin"
    else
        echo "Nothing to rebuild."
    fi
}

[[ $# -eq 0 ]] && usage

case "$1" in
    build)    build_shaders && build_cpp ;;
    run)      "$BIN_OUT/$BINARY_NAME" ;;
    buildrun) build_shaders && build_cpp && "$BIN_OUT/$BINARY_NAME" ;;
    debug)    build_shaders && build_cpp && lldb "$BIN_OUT/$BINARY_NAME" -o run ;;
    *)        usage ;;
esac