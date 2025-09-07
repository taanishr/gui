//
//  LayoutBox.cpp
//  gui
//
//  Created by Taanish Reja on 9/4/25.
//

#include "layout_box.hpp"
#include "renderer.hpp"

void LayoutBox::setX(float x)
{
    this->x = x;
}

void LayoutBox::setY(float y)
{
    this->y = y;
}

void LayoutBox::setWidth(float w)
{
    this->width = w;
}

void LayoutBox::setHeight(float h)
{
    this->height = h;
}


void ShellLayout::update()
{
    elementBounds = {.topLeft = {x, y},
        .bottomRight ={x + width, y + height}};
    
    this->center = {x + width/2.0f, y + height/2.0f};
    this->halfExtent = {width / 2.0f, height / 2.0f};
    
}

void ShellLayout::setX(float x)
{
    this->x = x;
    update();
}

void ShellLayout::setY(float y)
{
    auto frameInfo = Renderer::active().getFrameInfo();
    this->y = frameInfo.height - y;
    update();
}

void ShellLayout::setWidth(float w)
{
    this->width = w;
    update();
}

void ShellLayout::setHeight(float h)
{
    this->height = h;
    update();
}


void ImageLayout::update()
{
    elementBounds = {.topLeft = {x, y},
        .bottomRight ={x + width, y + height}};
    
    this->center = {x + width/2.0f, y + height/2.0f};
    this->halfExtent = {width / 2.0f, height / 2.0f};
    
}

void ImageLayout::setX(float x)
{
    this->x = x;
    update();
}

void ImageLayout::setY(float y)
{
    auto frameInfo = Renderer::active().getFrameInfo();
    this->y = frameInfo.height - y;
    update();
}

void ImageLayout::setWidth(float w)
{
    this->width = w;
    update();
}

void ImageLayout::setHeight(float h)
{
    this->height = h;
    update();
}

