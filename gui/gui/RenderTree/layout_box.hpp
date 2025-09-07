//
//  LayoutBox.hpp
//  gui
//
//  Created by Taanish Reja on 9/4/25.
//

#pragma once
#include <concepts>
#include "metal_imports.hpp"
#include "bounds.hpp"

template <typename T>
concept Layout = requires(T t, float x, float y, float w, float h) {
    {t.width} -> std::convertible_to<float>;
    {t.height} -> std::convertible_to<float>;
    {t.x}-> std::convertible_to<float>;
    {t.y} -> std::convertible_to<float>;
    
    {t.setX(x)} -> std::same_as<void>;
    {t.setY(y)} -> std::same_as<void>;
    {t.setWidth(w)} -> std::same_as<void>;
    {t.setHeight(h)} -> std::same_as<void>;
};

struct LayoutBox {
    float width, height;
    float x, y;
    
    void setX(float x);
    void setY(float y);
    void setWidth(float w);
    void setHeight(float h);
};

struct ShellLayout {
    float width, height;
    float x, y;
    
    void update();
    
    void setX(float x);
    void setY(float y);
    void setWidth(float w);
    void setHeight(float h);
    
    Bounds elementBounds;
    simd_float2 halfExtent;
    simd_float2 center;
};

struct ImageLayout {
    float width, height;
    float x, y;
    
    void update();
    
    void setX(float x);
    void setY(float y);
    void setWidth(float w);
    void setHeight(float h);
    
    Bounds elementBounds;
    simd_float2 halfExtent;
    simd_float2 center;
};
