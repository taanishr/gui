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
#include <ranges>
#include <numeric>

// goals; tree that starts from null root (the primary view)
// inserted based on parent node and z that is relative to the parent

struct RenderNodeBase;

using EventHandler = std::function<void(RenderNodeBase&, const Event&)>;

struct RenderNodeComparator {
    bool operator()(const std::unique_ptr<RenderNodeBase>& a, const std::unique_ptr<RenderNodeBase>& b) const;
};

class RenderNodeBase {
public:
    virtual LayoutResult layout(LayoutContext& parentCtx) = 0;
    virtual void position(const PositionContext& parentCtx) = 0;
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
    
    LayoutResult layout(LayoutContext& parentCtx) {
        bool outOfFlow = this->layoutBox.position == Position::Absolute || this->layoutBox.position == Position::Fixed;
    
        
        if (outOfFlow) {
            this->layoutBox.computedWidth = this->layoutBox.width;
            this->layoutBox.computedHeight = this->layoutBox.height;
            return LayoutResult{.flowX = parentCtx.flowX, .flowY = parentCtx.flowY, .isInline = false};
        }
        
        LayoutContext ctx {};
        
        auto layoutRes = std::visit(Overloaded{
            [&](const Block& block){
                this->layoutBox.flowX = parentCtx.flowX;
                this->layoutBox.flowY = parentCtx.flowY;
                
                this->layoutBox.computedWidth = this->layoutBox.width > 0 ? this->layoutBox.width : parentCtx.width;
                
                ctx.width = this->layoutBox.computedWidth;
                
                int ci = 0;
                while (ci < children.size()) {
                    auto& child = children[ci];
                    auto childLayoutRes = child->layout(ctx);
                    ctx.flowY = childLayoutRes.flowY;
                    ctx.lastChildInline = childLayoutRes.isInline;
                    ci += 1;
                }
                
                this->layoutBox.computedHeight = this->layoutBox.height > 0 ? this->layoutBox.height : ctx.flowY - parentCtx.flowY;
                
                ctx.height = this->layoutBox.computedHeight;
    

                return LayoutResult{.flowX = ctx.flowX, .flowY = parentCtx.flowY + ctx.height, .isInline = false};
            },
            [&](const Inline& in) {
                auto intrinsicSize = this->drawable->measure();
                
                this->layoutBox.computedWidth = this->layoutBox.width > 0 ? this->layoutBox.width : intrinsicSize.width;
                this->layoutBox.computedHeight = this->layoutBox.height > 0 ? this->layoutBox.height : intrinsicSize.height;
                
                ctx.width = this->layoutBox.computedWidth;
                ctx.height = this->layoutBox.computedHeight;
                
                ctx.isInline = true;
                
                auto blockAncestor = static_cast<RenderNode*>(parent);
                
                auto it = std::find_if(blockAncestor->children.begin(), blockAncestor->children.end(), [&](auto& u_cptr){ return u_cptr.get() == this; });

                if (it != blockAncestor->children.end()) {
                    blockAncestor->children.insert(std::next(it), std::make_move_iterator(children.begin()), std::make_move_iterator(children.end()));
                }else {
                    blockAncestor->children.insert(blockAncestor->children.end(), std::make_move_iterator(children.begin()), std::make_move_iterator(children.end()));
                }
                
                children.clear();

                if (!parentCtx.lastChildInline) {
                    float runFlowX = parentCtx.flowX;
                    
                    if (parentCtx.isInline)
                        runFlowX += parentCtx.width;
                        
                    InlineRun newRun {.flowX = runFlowX, .flowY = parentCtx.flowY, .lineHeight = 0, .lineWidth = 0};
                    parentCtx.inlineRuns.push(newRun);
                }
                
                auto& lastRun = parentCtx.inlineRuns.top();
                
                this->layoutBox.flowX = lastRun.flowX;
                this->layoutBox.flowY = lastRun.flowY;
                
                lastRun.flowX += this->layoutBox.computedWidth;
                lastRun.lineHeight = std::max(this->layoutBox.computedHeight, lastRun.lineHeight);
                lastRun.lineWidth += this->layoutBox.computedWidth;

                return LayoutResult{.flowX = parentCtx.flowX, .flowY = lastRun.flowY + lastRun.lineHeight, .isInline = true};
            }
        }, this->layoutBox.display);
        
        return layoutRes;
    }
    
    void position(const PositionContext& parentCtx) {
        switch (this->layoutBox.position) {
            case Position::Static:
                this->layoutBox.computedX = parentCtx.drawX + this->layoutBox.flowX;
                this->layoutBox.computedY = parentCtx.drawY + this->layoutBox.flowY;
                break;
            case Position::Relative:
                this->layoutBox.computedX = parentCtx.drawX + this->layoutBox.flowX + this->layoutBox.x;
                this->layoutBox.computedY = parentCtx.drawY + this->layoutBox.flowY + this->layoutBox.y;
                break;
            case Position::Fixed:
                this->layoutBox.computedX = this->layoutBox.x;
                this->layoutBox.computedY = this->layoutBox.y;
                break;
            case Position::Absolute:
                auto ancestor = static_cast<RenderNode*>(parent);
                while (ancestor->parent and ancestor->layoutBox.position == Position::Static) {
                    ancestor = static_cast<RenderNode*>(ancestor->parent);
                }
                
                this->layoutBox.computedX = ancestor->layoutBox.computedX + this->layoutBox.x;
                this->layoutBox.computedY = this->layoutBox.y;
        }
        
        PositionContext ctx {};
        
        ctx.drawX = this->layoutBox.computedX;
        ctx.drawY = this->layoutBox.computedY;
        
        for (auto& child : this->children) {
            child->position(ctx);
        }

        
        this->layoutBox.sync();
    }
    

    void update() {
        if (drawable)
            drawable->update(layoutBox);
    
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
    void position();
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

