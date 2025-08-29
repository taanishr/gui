//
//  MTKTextures.swift
//  MTKTextureBindings
//
//  Created by Taanish Reja on 1/12/25.
//

import Foundation
import Metal
import MetalKit

public func createMTKTextureLoader(devicePtr: UnsafeMutableRawPointer) -> UnsafeMutableRawPointer {
    let device = Unmanaged<MTLDevice>.fromOpaque(devicePtr).takeUnretainedValue();
    let loader = MTKTextureLoader(device: device);
    let loaderPtr = UnsafeMutableRawPointer(Unmanaged.passRetained(loader).toOpaque());
    return loaderPtr;
}

public func releaseMTKTextureLoader(loaderPtr: UnsafeMutableRawPointer) {
    let loader = Unmanaged<MTKTextureLoader>.fromOpaque(loaderPtr);
    loader.release();
}

public func createTexture(loaderPtr: UnsafeMutableRawPointer, filePath: UnsafePointer<CChar>) -> UnsafeMutableRawPointer?
{
    let loader = Unmanaged<MTKTextureLoader>.fromOpaque(loaderPtr).takeUnretainedValue();
    let fileURL = URL(fileURLWithPath: String(cString: filePath));
    
    let options: [MTKTextureLoader.Option: Any] = [
        .textureUsage: MTLTextureUsage.shaderRead.rawValue,
        .textureStorageMode: MTLStorageMode.private.rawValue
    ];
    
    guard let texture = try? loader.newTexture(URL: fileURL, options: options)
    else {
        return nil;
    }
    
    let texturePtr = UnsafeMutableRawPointer(Unmanaged.passRetained(texture).toOpaque());
    return texturePtr; // must transfer ownership using NS::SharedPtr in C++
}
