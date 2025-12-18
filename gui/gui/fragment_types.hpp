//
//  fragment_types.hpp
//  gui
//
//  Created by Taanish Reja on 11/20/25.
//

#include <vector>
#include "buffer_allocator.hpp"

// idea of a drawable
// Drawables create vector<Atom>; they generate the unpositioned base units
    // Atoms share a local position buffer
    // each atom thus has the same buffer handle and specifies offset + length
    // each atom also stores width + height
    // FragmentTemplate is a convenient wrapper over this with instrinsics (e.g. width/height)

// Layout engine
// Transforms Fragment Template into Placed Fragment Template
    // loops over atoms; places each one individually

struct AtomPoint {
    simd_float2 point;
    unsigned int id;
};

struct Atom {
    BufferHandle atomBufferHandle; // local position buffer
    size_t offset;
    size_t length;
    
    float width;
    float height;
};

struct AtomPlacement {
    BufferHandle placementBufferHandle;
    float x;
    float y;
};

template <typename T>
struct Uniforms {
    BufferHandle uniformsBufferHandle;
    T uniforms;
};

template <typename T>
struct FragmentTemplate {
    std::vector<Atom> atoms; // individual atoms
    std::vector<AtomPlacement> atomPlacements;
    float width; // width of fragment
    float height; // height of fragment

    Uniforms<T> uniforms;
};



