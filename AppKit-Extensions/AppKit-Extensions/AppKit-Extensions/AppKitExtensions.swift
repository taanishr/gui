//
//  appKitExtensions.swift
//  AppKit-Extensions
//
//  Created by Taanish Reja on 7/21/25.
//

import AppKit
import MetalKit

public func setTitleBarTransparent(windowPtr: UnsafeMutableRawPointer) {
    let window = Unmanaged<NSWindow>.fromOpaque(windowPtr).takeUnretainedValue();
    window.titlebarAppearsTransparent = true;
    window.styleMask.insert(.fullSizeContentView);
    print(window.titlebarAppearsTransparent);
}

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

public func setMaximumDrawableCount(viewPtr: UnsafeMutableRawPointer, count: CInt) {
    let view = Unmanaged<MTKView>.fromOpaque(viewPtr).takeUnretainedValue();
    
    if let CAMetalLayer = view.layer as? CAMetalLayer {
        CAMetalLayer.maximumDrawableCount = Int(count);
    }
}
