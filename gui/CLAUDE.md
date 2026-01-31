# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

Build and run via the `ship.sh` script:

```bash
./ship.sh build      # Build the project (Xcode, Debug config)
./ship.sh run        # Run the built binary
./ship.sh buildrun   # Build and run
./ship.sh debug      # Build and launch with lldb
```

The binary is output to `./build/Build/Products/Debug/gui`.

## Project Overview

This is a C++23 GPU-accelerated GUI framework for macOS using Metal. It implements a declarative, React-like component model with a custom layout engine.

**Key dependencies:**
- Metal/MetalKit (GPU rendering)
- FreeType (font rasterization, via Homebrew at `/opt/homebrew/opt/freetype`)
- metal-cpp and metal-cpp-extensions (`~/metal-cpp`, `~/metal-cpp-extensions`)
- Custom AppKit/MTK extension libraries (`~/projects/gui/AppKit-Extensions-Library`, `~/projects/gui/MTK-Extensions-Library`)

## Architecture

### Element System

Three element types in `gui/`: `Div` (boxes), `Text` (glyphs), `Image` (textures).

Each element must satisfy the `ElementType` concept (`element.hpp`):
- `getDescriptor()` - styling/properties
- `getFragment()` - GPU buffer storage
- `request(target, payload)` - descriptor query system
- Type aliases: `StorageType`, `DescriptorType`, `UniformsType`

Elements are type-erased via `ElementBase` → `Element<E,P>` for heterogeneous trees.

### 6-Phase Rendering Pipeline

Defined in `new_arch.hpp`, each processor implements:

1. **measure** - Resolve percentages, compute explicit sizes from `Constraints`
2. **atomize** - Convert to drawable atoms (glyphs, quads), write to GPU buffers
3. **layout** - Position atoms using layout engine (inline/block, line boxes, margin collapse)
4. **place** - Assign final x,y coordinates to atom placements
5. **finalize** - Create uniform buffers for shaders
6. **encode** - Issue Metal draw commands

### Layout Engine

Core concepts from `new_arch.hpp`:
- **Constraints**: viewport bounds, cursor, available width/height
- **Display modes**: Block (new line) vs Inline (flow with siblings)
- **Position modes**: Relative (normal flow), Absolute (positioned ancestor), Fixed (viewport)
- **Atoms**: indivisible content units with `ownerNodeId`, `intrinsicRect`, `styleId`
- **Line boxes**: wrapping containers for inline content; greedy line-breaking

Margin collapse rules apply to first/last children and adjacent blocks.

### GPU Memory

- `DrawableBuffer` / `DrawableBufferAllocator` - GPU buffer management (`buffer_allocator.hpp`)
- `FrameBufferedBuffer` - Ring buffer for multi-frame buffering (`frame_buffered_buffer.hpp`)
- Each element maintains: `atomsBuffer`, `placementsBuffer`, `uniformsBuffer`

### Scene Tree

`TreeNode` (`element.hpp`) manages:
- Parent/child hierarchy
- Cached results from all 6 pipeline phases
- Z-index (local and global)
- Event dispatching with bubbling (`addEventListener`, `dispatch`)

### Metal Shaders

Located in `gui/`:
- `common.metal` - shared frame info struct
- `div_shaders.metal` - box rendering with rounded corners
- `text_shaders.metal` - glyph rendering via SDFs
- `image_shaders.metal` - textured quads

## Code Patterns

- Heavy use of C++23 concepts for compile-time interface enforcement
- SIMD types (`simd_float2`, `simd_float4`) for GPU-compatible data
- Move semantics throughout; buffers are move-only
- `std::any` for type erasure at the `finalize` → `encode` boundary
