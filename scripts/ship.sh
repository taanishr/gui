#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$ROOT_DIR"

ENABLE_DEBUG_UI=0
ENABLE_PROFILE=0

usage() {
    cat <<EOF
Usage: scripts/ship.sh [--debug-ui] [--profile] [build|run|buildrun|debug|extensions|xcode|test]

Options:
  --debug-ui  Enable the compile-time debug inspector UI
  --profile   Build with optimization, debug symbols, and frame pointers

Commands:
  build       Configure and build the application
  run         Run the existing application build
  buildrun    Configure, build, and run the application
  debug       Configure and build, then run under lldb
  extensions  Build only the Swift AppKit/MTK extension libraries
  xcode       Compatibility alias for build
  test        Configure, build, and run the tests
EOF
    exit 1
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --debug-ui)
            ENABLE_DEBUG_UI=1
            shift
            ;;
        --profile)
            ENABLE_PROFILE=1
            shift
            ;;
        --)
            shift
            break
            ;;
        -*)
            usage
            ;;
        *)
            break
            ;;
    esac
done

[[ $# -eq 1 ]] || usage
COMMAND="$1"

if [[ $ENABLE_PROFILE -eq 1 && $ENABLE_DEBUG_UI -eq 1 ]]; then
    PRESET=profile-inspector
elif [[ $ENABLE_PROFILE -eq 1 ]]; then
    PRESET=profile
elif [[ $ENABLE_DEBUG_UI -eq 1 ]]; then
    PRESET=inspector
else
    PRESET=debug
fi

configure() {
    cmake --preset "$PRESET" -S "$ROOT_DIR"
}

build() {
    configure
    cmake --build --preset "$PRESET"
}

binary="$ROOT_DIR/build/$PRESET/gui"

case "$COMMAND" in
    build|xcode)
        build
        ;;
    run)
        [[ -x "$binary" ]] || {
            echo "Missing executable: $binary"
            echo "Run: scripts/ship.sh build"
            exit 1
        }
        exec "$binary"
        ;;
    buildrun)
        build
        exec "$binary"
        ;;
    debug)
        build
        exec lldb "$binary" -o run
        ;;
    extensions)
        configure
        cmake --build --preset "$PRESET" --target gui_apple_extensions
        ;;
    test)
        configure
        cmake --build --preset "$PRESET" --target gui_bidi_test
        ctest --preset "$PRESET"
        ;;
    *)
        usage
        ;;
esac
