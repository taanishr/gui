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
#include <variant>

enum class FlexDirection {
    Row,
    Column
};


enum class FlexGrowth {
    None,
    Shrink,
    Grow
};

struct Flex {
    Flex();
    
    FlexDirection direction;
    FlexGrowth growth;
};

struct Block {};

using Display = std::variant<Block>;

enum class Position {
    Static,
    Relative,
    Fixed,
    Absolute
};

template <typename T>
concept Layout = requires(T t, float x, float y, float w, float h) {
    { t.width } -> std::convertible_to<float>;
    { t.height } -> std::convertible_to<float>;
    { t.intrinsicWidth } -> std::convertible_to<float>;
    { t.intrinsicHeight } -> std::convertible_to<float>;
    { t.computedWidth } -> std::convertible_to<float>;
    { t.computedHeight } -> std::convertible_to<float>;
    { t.x }-> std::convertible_to<float>;
    { t.y } -> std::convertible_to<float>;
    { t.computedX } -> std::convertible_to<float>;
    { t.computedY } -> std::convertible_to<float>;
    { t.display } -> std::convertible_to<Display>;
    { t.position } -> std::convertible_to<Position>;
    
    { t.setX(x) } -> std::same_as<void>;
    { t.setY(y) } -> std::same_as<void>;
    { t.setWidth(w) } -> std::same_as<void>;
    { t.setHeight(h) } -> std::same_as<void>;
    { t.sync() } -> std::same_as<void>;
};

struct LayoutContext {
    float x;
    float y;
    float height;
    float width;
};

struct DefaultLayout {
    float width, height;
    float intrinsicWidth, intrinsicHeight;
    float computedWidth, computedHeight;
    float x, y;
    float computedX, computedY;
    Display display;
    Position position;
    
    void setX(float x);
    void setY(float y);
    void setWidth(float w);
    void setHeight(float h);
    void sync();
};

struct ShellLayout {
    float width, height;
    float intrinsicWidth, intrinsicHeight;
    float computedWidth, computedHeight;
    float x, y;
    float computedX, computedY;
    Display display;
    Position position;
    
    void setX(float x);
    void setY(float y);
    void setWidth(float w);
    void setHeight(float h);
    void setBlock();
    void sync();
    
    Bounds elementBounds;
    simd_float2 halfExtent;
    simd_float2 center;
    
};

struct ImageLayout {
    float width, height;
    float intrinsicWidth, intrinsicHeight;
    float computedWidth, computedHeight;
    float x, y;
    float computedX, computedY;
    Display display;
    Position position;
    
    void setX(float x);
    void setY(float y);
    void setWidth(float w);
    void setHeight(float h);
    void sync();
    
    Bounds elementBounds;
    simd_float2 halfExtent;
    simd_float2 center;
};

struct TextLayout {
    float width, height;
    float intrinsicWidth, intrinsicHeight;
    float computedWidth, computedHeight;
    float x, y;
    float computedX, computedY;
    Display display;
    Position position;
    
    void update();
    
    void setX(float x);
    void setY(float y);
    void setWidth(float w);
    void setHeight(float h);
    void sync();
    
    Bounds elementBounds;
};
