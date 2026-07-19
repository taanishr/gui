//
//  MTKTextures.swift
//  MTKTextureBindings
//
//  Created by Taanish Reja on 1/12/25.
//

import Foundation
import ImageIO
import Metal
import MetalKit

@_cdecl("createMTKTextureLoader")
public func createMTKTextureLoader(devicePtr: UnsafeMutableRawPointer) -> UnsafeMutableRawPointer {
    let device = Unmanaged<MTLDevice>.fromOpaque(devicePtr).takeUnretainedValue();
    let loader = MTKTextureLoader(device: device);
    let loaderPtr = UnsafeMutableRawPointer(Unmanaged.passRetained(loader).toOpaque());
    return loaderPtr;
}

@_cdecl("releaseMTKTextureLoader")
public func releaseMTKTextureLoader(loaderPtr: UnsafeMutableRawPointer) {
    let loader = Unmanaged<MTKTextureLoader>.fromOpaque(loaderPtr);
    loader.release();
}

@_cdecl("createDownsampledTexture")
public func createDownsampledTexture(
    loaderPtr: UnsafeMutableRawPointer,
    filePath: UnsafePointer<CChar>,
    targetPixelWidth: UInt32,
    targetPixelHeight: UInt32
) -> UnsafeMutableRawPointer? {
    let loader = Unmanaged<MTKTextureLoader>.fromOpaque(loaderPtr).takeUnretainedValue()
    let fileURL = URL(fileURLWithPath: String(cString: filePath))

    guard targetPixelWidth > 0, targetPixelHeight > 0,
          let source = CGImageSourceCreateWithURL(fileURL as CFURL, nil) else {
        return nil
    }

    let thumbnailOptions: [CFString: Any] = [
        kCGImageSourceCreateThumbnailFromImageAlways: true,
        kCGImageSourceCreateThumbnailWithTransform: true,
        kCGImageSourceThumbnailMaxPixelSize: Int(max(targetPixelWidth, targetPixelHeight))
    ]

    guard let image = CGImageSourceCreateThumbnailAtIndex(
        source,
        0,
        thumbnailOptions as CFDictionary
    ) else {
        return nil
    }

    guard let context = CGContext(
        data: nil,
        width: image.width,
        height: image.height,
        bitsPerComponent: 8,
        bytesPerRow: 0,
        space: CGColorSpaceCreateDeviceRGB(),
        bitmapInfo: CGImageAlphaInfo.premultipliedLast.rawValue
    ) else {
        return nil
    }
    context.draw(image, in: CGRect(x: 0, y: 0, width: image.width, height: image.height))

    guard let rgbaImage = context.makeImage() else {
        return nil
    }

    let textureOptions: [MTKTextureLoader.Option: Any] = [
        .textureUsage: MTLTextureUsage.shaderRead.rawValue,
        .textureStorageMode: MTLStorageMode.private.rawValue,
        .origin: MTKTextureLoader.Origin.topLeft
    ]

    guard let texture = try? loader.newTexture(cgImage: rgbaImage, options: textureOptions) else {
        return nil
    }

    return UnsafeMutableRawPointer(Unmanaged.passRetained(texture).toOpaque())
}
