//
//  render_tree.hpp
//  gui
//
//  Created by Taanish Reja on 8/21/25.
//


// ownership dynamics
// render tree owns all nodes
// nodes merely hold ptr to other nodes

// when we add or delete a node, the render tree handles this, not nodes

#pragma once
#include "metal_imports.hpp"
#include <memory>
#include <set>
#include "drawable.hpp"
#include <print>
#include <algorithm>
#include <any>
#include <functional>
#include "events.hpp"
#include "sdf_helpers.hpp"
#include "ui.hpp"
#include "layout.hpp"
#include "overloaded.hpp"

// goals; tree that starts from null root (the primary view)
// inserted based on parent node and z that is relative to the parent

struct RenderNodeBase;

using EventHandler = std::function<void(RenderNodeBase&, const Event&)>;

struct RenderNodeComparator {
    bool operator()(const std::unique_ptr<RenderNodeBase>& a, const std::unique_ptr<RenderNodeBase>& b) const;
};

class RenderNodeBase {
public:
    virtual simd_float2 layout(const LayoutContext& parentCtx) = 0;
    virtual void update() = 0;
    virtual void encode(MTL::RenderCommandEncoder* encoder) const = 0;
    virtual void render(MTL::RenderCommandEncoder* encoder) const = 0;
    virtual void setZIndex(int zIndex) = 0;
    virtual void dispatch(const Event& event) = 0;
    
    RenderNodeBase* parent;
    std::vector<std::unique_ptr<RenderNodeBase>> children;
    int localZIndex;
    int globalZIndex;
    uint64_t insertionId;
    uintptr_t pipelineId;
    std::unordered_map<EventType, std::vector<EventHandler>> handlers;
};

template <typename DrawableType, typename LayoutType = DefaultLayout>
    requires Drawable<DrawableType, LayoutType>
class RenderNode : public RenderNodeBase {
public:
    RenderNode() {}
    
    simd_float2 layout(const LayoutContext& parentCtx) {
        auto intrinsicSize = drawable->measure();
        this->layoutBox.intrinsicWidth, this->layoutBox.intrinsicHeight = intrinsicSize.width, intrinsicSize.height;
        
        
        auto finalCursor = std::visit(Overloaded{
            [&](const Block& block){
                LayoutContext ctx {};
                
                ctx.x = parentCtx.x;
                ctx.y = parentCtx.y;
                
                this->layoutBox.computedWidth = this->layoutBox.width > 0 ? this->layoutBox.width : parentCtx.width;
                ctx.width = this->layoutBox.computedWidth;
                

//                std::println("current drawable ptr: {}, parentCtx.y: {}", reinterpret_cast<void*>(this->drawable.get()), parentCtx.y);
                
                float childrenHeight = 0;
                
                for (const auto& child : children) {
                    auto cursor = child->layout(ctx);
                    ctx.y -= cursor.y;
                }
                
                std::println("current drawable ptr: {}, ctx.y {}", reinterpret_cast<void*>(this->drawable.get()), ctx.y);
                
                
                this->layoutBox.computedHeight = this->layoutBox.height > 0 ? this->layoutBox.height : parentCtx.y - ctx.y;
                
                ctx.height = this->layoutBox.computedHeight;
                
                this->layoutBox.computedX = parentCtx.x;
                this->layoutBox.computedY = ctx.y - this->layoutBox.computedHeight;
            

                this->layoutBox.sync();
                
                return simd_float2{this->layoutBox.computedX, this->layoutBox.computedY};
            },
        }, this->layoutBox.display);

        
        return finalCursor;

        
        //                this->layoutBox.computedWidth = resolveDimension(this->layoutBox.width, intrinsicSize.width);
        //                this->layoutBox.computedHeight = resolveDimension(this->layoutBox.height, intrinsicSize.height)
        
//        this->layoutBox.computedX = parentCtx.x + this->layoutBox.x;
//        this->layoutBox.computedY = parentCtx.y - this->layoutBox.y - this->layoutBox.height;
//        this->layoutBox.sync();
//
//        
//        LayoutContext ctx {};
//        ctx.x = this->layoutBox.computedX;
//        ctx.y = this->layoutBox.computedY;
//        ctx.width = this->layoutBox.computedWidth;
//        ctx.height = this->layoutBox.computedHeight;
//
//        
//        // need to fix this and modify positioning/layout to work based on the display mode foremost, not relative/absolute defaults
//        for (const auto& child : children)
//            child->layout(ctx);
    }
    

    void update() {
        if (drawable)
            drawable->update(layoutBox);
    
//        std::sort(children.begin(), children.end(), RenderNodeComparator{});
        for (const auto& child: children)
            child->update();
    }

    void encode(MTL::RenderCommandEncoder* encoder) const {
        if (drawable)
            drawable->encode(encoder);
    }

    void render(MTL::RenderCommandEncoder* encoder) const {
        encode(encoder);
    
        for (const auto& child: children)
            child->render(encoder);
    }

    void setZIndex(int zIndex) {
//        this->zIndex = this->parent->zIndex + zIndex;
        this->localZIndex = zIndex;
    }

    template <EventType E, typename F>
    void addEventHandler(F&& f) {
        EventHandler wrapper = [fn = std::forward<F>(f)](RenderNodeBase& self, const Event& event){
            auto& typedSelf = static_cast<RenderNode<DrawableType, LayoutType>&>(self);
            
            if constexpr (E == EventType::Click) {
                auto& mousePayload = std::get<MousePayload>(event.payload);

                if (!typedSelf.drawable->contains({mousePayload.x, mousePayload.y}))
                    return;
            }

            fn(typedSelf, std::get<event_payload_t<E>>(event.payload));
        };

        handlers[E].push_back(std::move(wrapper));
    }

    void dispatch(const Event& event) {
        auto it = handlers.find(event.type);
    
        if (it != handlers.end())
            for (auto& h : it->second) h(*this, event);
    
        for (auto& child : children) child->dispatch(event);
    }

    std::unique_ptr<DrawableType> drawable;
    LayoutType layoutBox;
};

class RenderTree {
public:
    RenderTree();

    void flatten(RenderNodeBase* node, int parentZ);
    
    void layout(const FrameInfo& frameSize);
    void update();
    void render(MTL::RenderCommandEncoder* encoder) const;

    template <typename DrawableType, typename LayoutType = DefaultLayout>
    RenderNode<DrawableType, LayoutType>* insertNode(std::unique_ptr<DrawableType> drawable, RenderNodeBase* parent)
    {
        auto newNode = std::make_unique<RenderNode<DrawableType, LayoutType>>();
        newNode->parent = parent;
        newNode->localZIndex = 0;
        newNode->globalZIndex = parent->globalZIndex;
        newNode->pipelineId = reinterpret_cast<std::uintptr_t>(drawable->getPipeline());
        newNode->drawable = std::move(drawable);
        newNode->insertionId = nextInsertionId;
        
        auto nodePtr = newNode.get();
    
        parent->children.push_back(std::move(newNode));
        
        nodeMap[nextInsertionId] = nodePtr;
        ++nodes;
        ++nextInsertionId;

        return nodePtr;
    }

    void deleteNode(RenderNodeBase* node);

    void reparent(RenderNodeBase* node, RenderNodeBase* newParent);

    void dispatch(const Event& event);
    
    std::unique_ptr<RenderNode<RootDrawable>> root;
    std::unordered_map<uint64_t, RenderNodeBase*> nodeMap;
    std::vector<RenderNodeBase*> drawList;
    
    // to do
    // - flatten render node base; sorted by global z index (so we don't have cousin drift)
    // - only resort when children z changes (dirty flag)
    // - secondary sort (same z buffer) -> do it by render pipeline key
    
    // structure:
    // - have update flatten draw list
    // - track both local and global z index
    // - map pipeline keys somehow
    // - have encode just run through the draw list


    size_t nodes;
    uint64_t nextInsertionId;
    
    static RenderTree* current;
};

