//
//  appKitExtensions.swift
//  AppKit-Extensions
//
//  Created by Taanish Reja on 7/21/25.
//

import AppKit
import MetalKit

@_cdecl("setTitleBarTransparent")
public func setTitleBarTransparent(windowPtr: UnsafeMutableRawPointer) {
    let window = Unmanaged<NSWindow>.fromOpaque(windowPtr).takeUnretainedValue();
    window.titlebarAppearsTransparent = true;
    window.styleMask.insert(.fullSizeContentView);
}

@_cdecl("setWindowTransparent")
public func setWindowTransparent(windowPtr: UnsafeMutableRawPointer) {
    let window = Unmanaged<NSWindow>.fromOpaque(windowPtr).takeUnretainedValue();
    
    window.isOpaque = false;
    window.backgroundColor = .clear;
    
    if let contentView = window.contentView as? MTKView {
        contentView.wantsLayer = true;
        contentView.layer?.isOpaque = false;
        contentView.layer?.backgroundColor = .clear;
    }
}

@_cdecl("setMaximumDrawableCount")
public func setMaximumDrawableCount(viewPtr: UnsafeMutableRawPointer, count: CInt) {
    let view = Unmanaged<MTKView>.fromOpaque(viewPtr).takeUnretainedValue();
    
    if let CAMetalLayer = view.layer as? CAMetalLayer {
        CAMetalLayer.maximumDrawableCount = Int(count);
    }
}

@_cdecl("setSyncEnabled")
public func setSyncEnabled(viewPtr: UnsafeMutableRawPointer, enabled: CBool) {
    let view = Unmanaged<MTKView>.fromOpaque(viewPtr).takeUnretainedValue();
    
    if let CAMetalLayer = view.layer as? CAMetalLayer {
        CAMetalLayer.displaySyncEnabled = enabled;
    }
}

@_cdecl("getContentScaleFactor")
public func getContentScaleFactor(viewPtr: UnsafeMutableRawPointer) -> CFloat {
    let view = Unmanaged<MTKView>.fromOpaque(viewPtr).takeUnretainedValue();
    
    if let window = view.window {
        return CFloat(window.backingScaleFactor);
    }
    
    return 1.0; // default fall through
}
