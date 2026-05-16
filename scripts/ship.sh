#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

BINARY_NAME="gui"
SRC_DIR="$ROOT_DIR/src"
BUILD_DIR="$ROOT_DIR/build"
BIN_OUT="$BUILD_DIR/bin"
OBJ_DIR="$BIN_OUT/obj"
APPLE_EXT_BUILD="$BUILD_DIR/apple-extensions"
MODULE_CACHE_DIR="$BUILD_DIR/module-cache"

XCODEBUILD="/Applications/Xcode.app/Contents/Developer/usr/bin/xcodebuild"
SDK_PATH="/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk"
METAL_CPP="/Users/treja/metal-cpp"
METAL_CPP_EXT="/Users/treja/metal-cpp-extensions"
FREETYPE_INC="/opt/homebrew/opt/freetype/include/freetype2"
RESVG_INC="/opt/homebrew/opt/resvg/include/resvg"
LIB_RESVG="/opt/homebrew/opt/resvg/lib"

LIB_FREETYPE="/opt/homebrew/Cellar/freetype/2.13.3/lib/libfreetype.a"
LIB_MTK_EXT="$APPLE_EXT_BUILD/MTK-Extensions"
LIB_APPKIT_EXT="$APPLE_EXT_BUILD/AppKit-Extensions"

CXXFLAGS=(
    -std=c++23
    -isysroot "$SDK_PATH"
    -fno-objc-arc
    -D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_NONE
    -stdlib=libc++
    -I"$SRC_DIR"
    -I"$METAL_CPP"
    -I"$METAL_CPP_EXT"
    -I"$FREETYPE_INC"
    -I"$RESVG_INC"
    -O0 -g
)

usage() {
    cat <<EOF
Usage: scripts/ship.sh [build|run|buildrun|debug|extensions|xcode]

Commands:
  build       Build Swift extensions, Metal shaders, and C++ executable
  run         Run build/bin/gui
  buildrun   Build, then run build/bin/gui
  debug      Build, then run build/bin/gui under lldb
  extensions Build only the Swift AppKit/MTK extension static libraries
  xcode      Build the main gui.xcodeproj target with xcodebuild
EOF
    exit 1
}

needs_rebuild() {
    local src="$1" out="$2" dep="${3:-}"
    [[ ! -f "$out" ]] && return 0
    [[ "$src" -nt "$out" ]] && return 0
    [[ -n "$dep" && ! -f "$dep" ]] && return 0
    [[ -z "$dep" || ! -f "$dep" ]] && return 1

    while IFS= read -r line; do
        line="${line%\\}"
        line="${line#"${line%%[![:space:]]*}"}"
        line="${line#*: }"
        for hdr in $line; do
            [[ -f "$hdr" && "$hdr" -nt "$out" ]] && return 0
        done
    done < "$dep"

    return 1
}

build_extensions() {
    mkdir -p "$APPLE_EXT_BUILD" "$BUILD_DIR/xcode-derived"

    echo "Building AppKit extension..."
    "$XCODEBUILD" \
        -project "$ROOT_DIR/apple-extensions/AppKit/AppKit-Extensions.xcodeproj" \
        -scheme AppKit-Extensions \
        -configuration Debug \
        -derivedDataPath "$BUILD_DIR/xcode-derived/AppKit" \
        CODE_SIGNING_ALLOWED=NO \
        build

    echo "Building MTK extension..."
    "$XCODEBUILD" \
        -project "$ROOT_DIR/apple-extensions/MTK/MTK-Extensions.xcodeproj" \
        -scheme MTK-Extensions-Library \
        -configuration Debug \
        -derivedDataPath "$BUILD_DIR/xcode-derived/MTK" \
        CODE_SIGNING_ALLOWED=NO \
        build
}

build_shaders() {
    echo "Compiling Metal shaders..."
    local metal metallib
    metal="$(xcrun -sdk macosx --find metal)"
    metallib="$(xcrun -sdk macosx --find metallib)"

    mkdir -p "$BIN_OUT" "$MODULE_CACHE_DIR"
    local air_files=()

    for shader in "$SRC_DIR"/*.metal; do
        [[ -f "$shader" ]] || { echo "No .metal files found in $SRC_DIR"; return 1; }
        local air="$BIN_OUT/$(basename "$shader" .metal).air"
        if needs_rebuild "$shader" "$air"; then
            echo "  shader: ${shader#$ROOT_DIR/}"
            "$metal" -c "$shader" -fmodules-cache-path="$MODULE_CACHE_DIR" -o "$air"
            [[ -f "$air" ]] || { echo "Error: shader did not produce $air"; return 1; }
        fi
        air_files+=("$air")
    done

    local metallib_out="$BIN_OUT/default.metallib"
    local relink=0
    for air in "${air_files[@]}"; do
        [[ ! -f "$metallib_out" || "$air" -nt "$metallib_out" ]] && relink=1 && break
    done

    if [[ $relink -eq 1 ]]; then
        echo "  linking metallib..."
        "$metallib" "${air_files[@]}" -o "$metallib_out"
        [[ -f "$metallib_out" ]] || { echo "Error: metallib did not produce $metallib_out"; return 1; }
    fi

    echo "Shaders done."
}

build_cpp() {
    mkdir -p "$OBJ_DIR"

    if [[ ! -f "$LIB_MTK_EXT" || ! -f "$LIB_APPKIT_EXT" ]]; then
        echo "Missing Swift extension libraries. Run: scripts/ship.sh extensions"
        return 1
    fi

    local llvm_prefix
    llvm_prefix="$(brew --prefix llvm)"

    local objs=()
    local srcs_to_compile=()
    local any_rebuilt=0

    for src in "$SRC_DIR"/*.cpp; do
        [[ -f "$src" ]] || continue
        local base obj dep
        base="$(basename "$src" .cpp)"
        obj="$OBJ_DIR/${base}.o"
        dep="$OBJ_DIR/${base}.d"
        objs+=("$obj")

        if needs_rebuild "$src" "$obj" "$dep"; then
            srcs_to_compile+=("$src")
            any_rebuilt=1
        fi
    done

    if [[ ${#srcs_to_compile[@]} -gt 0 ]]; then
        local nproc fail_marker
        nproc="$(getconf _NPROCESSORS_ONLN 2>/dev/null || true)"
        [[ -n "$nproc" && "$nproc" =~ ^[0-9]+$ && "$nproc" -gt 0 ]] || nproc=4
        fail_marker="$OBJ_DIR/.compile_failed"
        rm -f "$fail_marker"

        echo "Compiling ${#srcs_to_compile[@]} file(s) with $nproc jobs..."
        printf '%s\0' "${srcs_to_compile[@]}" | xargs -0 -P "$nproc" -I{} bash -c '
            src="$1"
            base="$(basename "$src" .cpp)"
            obj="'"$OBJ_DIR"'/${base}.o"
            dep="'"$OBJ_DIR"'/${base}.d"
            rel="${src#'"$ROOT_DIR"'/}"
            echo "  compile: $rel"
            if ! "'"$llvm_prefix"'/bin/clang++" '"$(printf " %q" "${CXXFLAGS[@]}")"' \
                -I"'"$llvm_prefix"'/include/c++/v1" \
                -MMD -MF "$dep" \
                -c "$src" -o "$obj"; then
                touch "'"$fail_marker"'"
            fi
        ' _ {}

        if [[ -f "$fail_marker" ]]; then
            rm -f "$fail_marker"
            echo "Error: compile failed"
            return 1
        fi
    fi

    local bin="$BIN_OUT/$BINARY_NAME"
    if [[ $any_rebuilt -eq 1 || ! -f "$bin" || "$LIB_MTK_EXT" -nt "$bin" || "$LIB_APPKIT_EXT" -nt "$bin" ]]; then
        echo "Linking $BINARY_NAME..."
        "$llvm_prefix/bin/clang++" \
            -std=c++23 \
            -isysroot "$SDK_PATH" \
            -fno-objc-arc \
            -stdlib=libc++ \
            "${objs[@]}" \
            "$LIB_MTK_EXT" "$LIB_APPKIT_EXT" "$LIB_FREETYPE" \
            -L"$llvm_prefix/lib/c++" \
            -Wl,-rpath,"$llvm_prefix/lib/c++" \
            -L"$LIB_RESVG" -lresvg \
            -Wl,-rpath,"$LIB_RESVG" \
            -L/opt/homebrew/lib -lpng -lbz2 -lz \
            -framework Metal -framework MetalKit -framework Foundation \
            -framework QuartzCore -framework AppKit -framework Cocoa \
            -L/usr/lib/swift \
            -lswiftCore -lswiftMetal -lswiftMetalKit \
            -lswiftFoundation -lswiftQuartzCore -lswiftAppKit \
            -Wl,-rpath,/usr/lib/swift \
            -lc++ \
            -o "$bin"
        echo "Build successful: $bin"
    else
        echo "Nothing to rebuild."
    fi
}

build_xcode() {
    build_extensions
    "$XCODEBUILD" \
        -project "$ROOT_DIR/gui.xcodeproj" \
        -scheme gui \
        -configuration Debug \
        -derivedDataPath "$BUILD_DIR/xcode-derived/gui" \
        CODE_SIGNING_ALLOWED=NO \
        build
}

run_binary() {
    local bin="$BIN_OUT/$BINARY_NAME"
    [[ -x "$bin" ]] || { echo "Missing executable: $bin"; return 1; }
    "$bin"
}

[[ $# -eq 0 ]] && usage

case "$1" in
    build) build_extensions && build_shaders && build_cpp ;;
    run) run_binary ;;
    buildrun) build_extensions && build_shaders && build_cpp && run_binary ;;
    debug) build_extensions && build_shaders && build_cpp && lldb "$BIN_OUT/$BINARY_NAME" -o run ;;
    extensions) build_extensions ;;
    xcode) build_xcode ;;
    *) usage ;;
esac
