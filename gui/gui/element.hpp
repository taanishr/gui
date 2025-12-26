#include "new_arch.hpp"
#include <concepts>
#include <type_traits>

namespace NewArch {

    template<
        typename T,
        typename D,
        typename S,
        template<typename> class F
    >
    concept ElementType = requires(T& t) {
        { t.getDescriptor() } -> std::same_as<D&>;
        { t.getFragment() }  -> std::same_as<F<S>&>;
    };

    template<
        typename P,
        typename D,
        typename S,
        template<typename> class Fr,
        template<typename> class Fn,
        typename U
    >
    concept ProcessorType =
        requires(
            P& proc,
            Fr<S>& fragment,
            Constraints& constraints,
            D& desc,
            Measured& measured,
            Atomized& atomized,
            Placed& placed,
            Fn<U>& finalized,
            MTL::RenderCommandEncoder* encoder
    ) {
        { proc.measure(fragment, constraints, desc) } -> std::same_as<Measured>;
        { proc.atomize(fragment, constraints, desc, measured) } -> std::same_as<Atomized>;
        { proc.place(fragment, constraints, desc, measured, atomized) } -> std::same_as<Placed>;
        { proc.finalize(fragment, constraints, desc, measured, atomized, placed) }
            -> std::same_as<Fn<U>>;
        { proc.encode(encoder, fragment, finalized) } -> std::same_as<void>;
    };


    // ughhh figure out type erasure route for finalized this annoys tf outta me bro
    struct ElementBase {
        virtual Measured measure(Constraints& constraints) = 0;
        virtual Atomized atomize(Constraints& constraints, Measured& measured) = 0;
        virtual Placed place(Constraints& constraints, Measured& measured, Atomized& atomized) = 0;
    };

    template <typename E, typename D, typename S, typename P, typename U> 
        requires ElementType<E, D, S, Fragment> && ProcessorType<P, D, S, Fragment, Finalized, U>
    struct Element : ElementBase {
        Element(E elem, P& proc);
        
        Measured measure(Constraints& constraints) {

        }

        Finalized<typename E::UniformsType> finalize(Constraints& constraints, Measured& measured, Atomized& atomized, Placed& placed);
        void encode(Finalized<typename E::UniformsType> finalized);

        E element;
        P& processor;
    };
}