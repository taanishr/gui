#include "metal_imports.hpp"
#include "new_arch.hpp"
#include <concepts>
#include <any>
#include <cstdint>

namespace NewArch {
    template<typename E>
    concept ElementType = requires(E& e) {
        e.getDescriptor();
        e.getFragment();
    };

    template<typename P, typename Fragment, typename Descriptor>
    concept ProcessorType = requires(
        P& proc,
        Fragment& fragment,
        Constraints& constraints,
        Descriptor& desc,
        Measured& measured,
        Atomized& atomized,
        Placed& placed,
        LayoutResult layout,
        MTL::RenderCommandEncoder* encoder
    ) {
        { proc.measure(fragment, constraints, desc) } -> std::same_as<Measured>;
        { proc.atomize(fragment, constraints, desc, measured) } -> std::same_as<Atomized>;
        { proc.layout(fragment, constraints, desc, measured, atomized) } -> std::same_as<LayoutResult>;
        { proc.place(fragment, constraints, desc, measured, atomized, layout) } -> std::same_as<Placed>;
        proc.finalize(fragment, constraints, desc, measured, atomized, placed);
        proc.encode(encoder, fragment, proc.finalize(fragment, constraints, desc, measured, atomized, placed));
    };

    // ughhh figure out type erasure route for finalized this annoys tf outta me bro
    struct ElementBase {
        virtual Measured measure(Constraints& constraints) = 0;
        virtual Atomized atomize(Constraints& constraints, Measured& measured) = 0;
        virtual LayoutResult layout(Constraints& constraints, Measured& measured, Atomized& atomized) = 0;
        virtual Placed place(Constraints& constraints, Measured& measured, Atomized& atomized, LayoutResult& layout) = 0;
        virtual std::any finalize(Constraints& constraints, Measured& measured, Atomized& atomized, Placed& placed) = 0;
        virtual void encode(MTL::RenderCommandEncoder* encoder, std::any& finalized) = 0;
        virtual ~ElementBase();
    };

    template <typename E, typename P, typename S, typename D> 
        requires ElementType<E> && ProcessorType<P, Fragment<S>, D>
    struct Element : ElementBase {
        Element(UIContext& ctx, E elem, P& proc):
            element{elem}, processor{proc}
        {}
        
        Measured measure(Constraints& constraints) {
            return processor.measure(element.getFragment(), constraints);
        }

        Atomized atomize(Constraints& constraints, Measured& measured) {
            return processor.atomize(element.getFragment(), constraints, measured);
        }


        LayoutResult layout(Constraints& constraints, Measured& measured, Atomized& atomized) {
            return processor.layout(element.getFragment(), constraints, measured, atomized);
        }

        Placed place(Constraints& constraints, Measured& measured, Atomized& atomized, LayoutResult& layout) {
            return processor.place(element.getFragment(), constraints, measured, atomized, layout);
        }

        std::any finalize(Constraints& constraints, Measured& measured, Atomized& atomized, Placed& placed) {
            auto finalized = processor.finalize(element.getFragment(), constraints,  measured, atomized, placed);
            auto finalizedErased = finalized;
            return finalizedErased;
        }

        void encode(MTL::RenderCommandEncoder* encoder, std::any finalizedErased) {
            auto finalized = std::any_cast<Finalized<typename E::UniformsType>>(finalizedErased);
            return processor.encode(encoder, finalized);
        }

        ~Element() {};

        E element;
        P& processor;
    };

    struct ComputationCache {
        std::unordered_map<uint64_t, Measured> measured;
        std::unordered_map<uint64_t, Atomized> atomized;
        std::unordered_map<uint64_t, LayoutResult> layouts;
        std::unordered_map<uint64_t, Placed> placed;
        std::unordered_map<uint64_t, std::any> finalized; 
    };


    struct TreeNode {
        template<typename E, typename P, typename S, typename D>
        TreeNode(E elem, P& processor)
            : element(std::make_unique<Element<E, P, S, D>>(std::move(elem), processor))
            , parent(nullptr)
            , id(nextId++)
        {}
        
        void add_child(std::unique_ptr<TreeNode> child) {
            child->parent = this;
            children.push_back(std::move(child));
        }
        
        template<typename E, typename P>
        TreeNode* create_child(E elem, P& processor) {
            auto child = std::make_unique<TreeNode>(std::move(elem), processor);
            auto* ptr = child.get();
            add_child(std::move(child));
            return ptr;
        }
        
        std::unique_ptr<ElementBase> element;
        TreeNode* parent = nullptr;
        std::vector<std::unique_ptr<TreeNode>> children;
        uint64_t id;
        
    private:
        static uint64_t nextId;
    };

    // pointers for raw views
    struct RenderTree {
        template<typename E, typename P>
        TreeNode* createRoot(E elem, P& processor) {
            elementTree = std::make_unique<TreeNode>(std::move(elem), processor);
            return elementTree.get();
        }
        
        TreeNode* getRoot() { return elementTree.get(); }
        
        void update(const FrameInfo& frameInfo);
        void render(MTL::RenderCommandEncoder* encoder);

    private:
        Constraints rootConstraints; // root constraints;
        simd_float2 rootCursor; // root cursor


        std::unique_ptr<TreeNode> elementTree;
        ComputationCache renderCache;
        LayoutEngine layoutEngine;
        
        void measurePhase(TreeNode* node, Constraints& constraints);
        void atomizePhase(TreeNode* node, Constraints& constraints);
        void layoutPhase(TreeNode* node, const FrameInfo& frameInfo, Constraints& constraints);
        void finalizePhase(TreeNode* node, Constraints& constraints);
    };

}
