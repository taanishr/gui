#!/bin/bash

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
        /Applications/Xcode.app/Contents/Developer/usr/bin/xcodebuild -project gui.xcodeproj -scheme "gui" -configuration Debug -sdk macosx -derivedDataPath ./build clean build | xcpretty
        ;;
    run)
        ./build/Build/Products/Debug/gui
        ;;
    buildrun)
        /Applications/Xcode.app/Contents/Developer/usr/bin/xcodebuild -project gui.xcodeproj -scheme "gui" -configuration Debug -sdk macosx -derivedDataPath ./build clean build | xcpretty
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


