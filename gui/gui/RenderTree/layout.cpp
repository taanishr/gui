//
//  LayoutBox.cpp
//  gui
//
//  Created by Taanish Reja on 9/4/25.
//

#include "layout.hpp"
#include "renderer.hpp"

Flex::Flex():
    direction{FlexDirection::Row},
    growth{FlexGrowth::None}
{}

void DefaultLayout::setX(float x)
{
    this->x = x;
}

void DefaultLayout::setY(float y)
{
    this->y = y;
}

void DefaultLayout::setWidth(float w)
{
    this->width = w;
}

void DefaultLayout::setHeight(float h)
{
    this->height = h;
}

void DefaultLayout::sync()
{}

void ShellLayout::setX(float x)
{
    this->x = x;
}

void ShellLayout::setY(float y)
{
    this->y = y;
}

void ShellLayout::setWidth(float w)
{
    this->width = w;
}

void ShellLayout::setHeight(float h)
{
    this->height = h;
}

void ShellLayout::setBlock() {
    this->display = Block{};
}

//void ShellLayout::setFlex() {
//    this->display = Flex{};
//}

void ShellLayout::sync()
{
    elementBounds = {.topLeft = {computedX, computedX},
        .bottomRight ={computedX + computedWidth, computedX + computedHeight}};
    
    this->center = {computedX + computedWidth/2.0f, computedY + computedHeight/2.0f};
    this->halfExtent = {computedWidth / 2.0f, computedHeight / 2.0f};
}

void ImageLayout::setX(float x)
{
    this->x = x;
}

void ImageLayout::setY(float y)
{
    this->y = y;
}

void ImageLayout::setWidth(float w)
{
    this->width = w;
}

void ImageLayout::setHeight(float h)
{
    this->height = h;
}

void ImageLayout::sync()
{
    elementBounds = {.topLeft = {computedX, computedX},
        .bottomRight ={computedX + computedWidth, computedX + computedHeight}};
    
    this->center = {computedX + computedWidth/2.0f, computedY + computedHeight/2.0f};
    this->halfExtent = {computedWidth / 2.0f, computedHeight / 2.0f};
}
    
void TextLayout::setX(float x)
{
    this->x = x;
}

void TextLayout::setY(float y)
{
    this->y = y;
}

void TextLayout::setWidth(float w)
{
    this->width = w;
}

void TextLayout::setHeight(float h)
{
    this->height = h;
}

void TextLayout::sync() {
    elementBounds = {.topLeft = {computedX, computedX},
        .bottomRight = {computedX + computedWidth, computedX + computedHeight}};
}
