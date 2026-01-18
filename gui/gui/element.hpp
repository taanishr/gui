#pragma once

#include "metal_imports.hpp"
#include "new_arch.hpp"
#include <concepts>
#include <any>
#include <cstdint>
#include <algorithm>
#include "frame_info.hpp"
#include <print>
#include <simd/vector_types.h>
#include "parallel.hpp"
#include "events.hpp"
#include "printers.hpp"

namespace NewArch {
    template<typename E>
    concept ElementType = requires(E& e) {
        e.getDescriptor();
        e.getFragment();

        typename E::StorageType;
        typename E::DescriptorType;
        typename E::UniformsType;
    };

    template<typename P, typename S, typename D, typename U>
    concept ProcessorType = requires(
        P& proc,
        Fragment<S>& fragment,
        Constraints& constraints,
        D& desc,
        Measured& measured,
        Atomized& atomized,
        Placed& placed,
        Finalized<U>& finalized,
        LayoutResult layout,
        MTL::RenderCommandEncoder* encoder
    ) {
        { proc.measure(fragment, constraints, desc) } -> std::same_as<Measured>;
        { proc.atomize(fragment, constraints, desc, measured) } -> std::same_as<Atomized>;
        { proc.layout(fragment, constraints, desc, measured, atomized) } -> std::same_as<LayoutResult>;
        { proc.place(fragment, constraints, desc, measured, atomized, layout) } -> std::same_as<Placed>;
        { proc.finalize(fragment, constraints, desc, measured, atomized, placed) } -> std::same_as<Finalized<U>>;
        proc.encode(encoder, fragment, finalized);
    };

    // ughhh figure out type erasure route for finalized this annoys tf outta me bro
    struct ElementBase {
        virtual Measured measure(Constraints& constraints) = 0;
        virtual Atomized atomize(Constraints& constraints, Measured& measured) = 0;
        virtual LayoutResult layout(Constraints& constraints, Measured& measured, Atomized& atomized) = 0;
        virtual Placed place(Constraints& constraints, Measured& measured, Atomized& atomized, LayoutResult& layout) = 0;
        virtual std::any finalize(Constraints& constraints, Measured& measured, Atomized& atomized, Placed& placed) = 0;
        virtual void encode(MTL::RenderCommandEncoder* encoder, std::any& finalized) = 0;
        virtual bool preciseHitTest(simd_float2 point, const LayoutResult& layout) {
            return true; 
        }

        virtual ~ElementBase() = 0;
    };

    template <ElementType E, typename P, typename S, typename D, typename U> 
        requires ProcessorType<P, S, D, U>
    struct Element : ElementBase {
        Element(UIContext& ctx, E&& elem, P& proc):
            element{std::move(elem)}, processor{proc}
        {}
        
        Measured measure(Constraints& constraints) override {
            return processor.measure(element.getFragment(), constraints, element.getDescriptor());
        }

        Atomized atomize(Constraints& constraints, Measured& measured) override {
            return processor.atomize(element.getFragment(), constraints, element.getDescriptor(), measured);
        }


        LayoutResult layout(Constraints& constraints, Measured& measured, Atomized& atomized) override {
            return processor.layout(element.getFragment(), constraints, element.getDescriptor(), measured, atomized);
        }

        Placed place(Constraints& constraints, Measured& measured, Atomized& atomized, LayoutResult& layout) override {
            return processor.place(element.getFragment(), constraints, element.getDescriptor(), measured, atomized, layout);
        }

        std::any finalize(Constraints& constraints, Measured& measured, Atomized& atomized, Placed& placed) override {
            auto finalized = processor.finalize(element.getFragment(), constraints, element.getDescriptor(), measured, atomized, placed);
            auto finalizedErased = finalized;
            return finalizedErased;
        }

        void encode(MTL::RenderCommandEncoder* encoder, std::any& finalizedErased) override {
            auto finalized = std::any_cast<Finalized<typename E::UniformsType>>(finalizedErased);
            return processor.encode(encoder, element.getFragment(), finalized);
        }

        ~Element() override {};

        E element;
        P& processor;
    };

    struct TreeNode;
    
    using EventHandler = std::function<void(const Event&)>;

    struct TreeNode {
        // using DivElem  = Element<Div<DivStorage>,  DivProcessor<...>, DivStorage, DivDescriptor, DivUniforms>;
        // using TextElem = Element<Text<...>, TextProcessor<...>, ...>;
        // using ImageElem= Element<Image<...>, ImageProcessor<...>, ...>;
        // using ElementVariant = std::variant<DivElem, TextElem, ImageElem>;


        template<ElementType E, typename P>
            requires ProcessorType<P, typename E::StorageType, typename E::DescriptorType, typename E::UniformsType>
        TreeNode(UIContext& ctx, E&& elem, P& processor)
            : element(std::make_unique<Element<E,P, typename E::StorageType, typename E::DescriptorType, typename E::UniformsType>>(ctx, std::move(elem), processor)),
            parent(nullptr),
            id(nextId++),
            localZIndex{0},
            globalZIndex{0}
        {}


        void addEventListener(EventType type, EventHandler handler) {
            eventHandlers[type].push_back(std::move(handler));
        }

        void dispatch(Event& event) {
            auto it = eventHandlers.find(event.type);
            if (it != eventHandlers.end()) {
                for (auto& handler : it->second) {
                    if (event.propagationStopped) break;
                    handler(event);
                }
            }
            
            if (!event.propagationStopped && parent) {
                parent->dispatch(event);
            }
        }

        void calculateGlobalZIndex(unsigned int parentGlobal) {
            globalZIndex = parentGlobal + localZIndex;
            
            for (auto& child : children) {
                child->calculateGlobalZIndex(globalZIndex);
            }
        }
        
        void attach_child(std::unique_ptr<TreeNode>&& child) {
            if (!child) return;
            child->parent = this;
            children.push_back(std::move(child));
        }

        bool contains(simd_float2 point) const {
            if (!layout.has_value()) return false;
            
            auto& box = layout->computedBox;
            // std::println("point: {}", point);
            
            // std::println("box.width: {}, box.height: {}", box.width, box.height);
            
            // std::println("topLeft: {} bottomRight: {}", simd_float2{box.x, box.y}, simd_float2{box.x + box.width , box.y+box.height});
            if (point.x < box.x || point.x > box.x + box.width ||
                point.y < box.y || point.y > box.y + box.height) {
                return false;
            }

            return element->preciseHitTest(point, layout.value());
        }

        
        std::unique_ptr<ElementBase> element;
        TreeNode* parent = nullptr;
        std::vector<std::unique_ptr<TreeNode>> children;
        uint64_t id;
        uint64_t localZIndex;
        uint64_t globalZIndex;

        std::optional<Measured> measured;
        std::optional<Atomized> atomized;
        std::optional<LayoutResult> layout;
        std::optional<Placed> placed;
        std::unordered_map<EventType, std::vector<EventHandler>> eventHandlers;
        std::any finalized;
        
    private:
        static uint64_t nextId;
    };

    std::vector<TreeNode*> collectAllNodes(TreeNode* root);

    // pointers for raw views
    struct RenderTree {
        template<ElementType E, typename P>
            requires ProcessorType<P, typename E::StorageType, typename E::DescriptorType, typename E::UniformsType>
        TreeNode* createRoot(UIContext& ctx, E elem, P& processor) {
            elementTree = std::make_unique<TreeNode>(ctx, std::move(elem), processor);
            return elementTree.get();
        }
        
        TreeNode* getRoot() { return elementTree.get(); }
        
        void update(const FrameInfo& frameInfo);
        void render(MTL::RenderCommandEncoder* encoder); 
        
        TreeNode* hitTestRecursive(TreeNode* node, simd_float2 point);
    private:
        Constraints rootConstraints; // root constraints;
        simd_float2 rootCursor; // root cursor

        std::unique_ptr<TreeNode> elementTree;
        LayoutEngine layoutEngine;
        
        void measurePhase(TreeNode* node, Constraints& constraints);
        void atomizePhase(TreeNode* node, Constraints& constraints);
        void layoutPhase(TreeNode* node, const FrameInfo& frameInfo, Constraints& constraints);
        void placePhase(TreeNode* node, const FrameInfo& frameInfo, Constraints& constraints);
        void finalizePhase(TreeNode* node, Constraints& constraints);
    };

}
