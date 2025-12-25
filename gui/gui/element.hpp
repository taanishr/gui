#include "new_arch.hpp"


namespace NewArch {


    // ughhh figure out type erasure route for finalized this annoys tf outta me bro
    struct ElementBase {
        virtual Measured measure(Constraints& constraints) = 0;
        virtual Atomized atomize(Constraints& constraints, Measured& measured) = 0;
        virtual Placed place(Constraints& constraints, Measured& measured, Atomized& atomized) = 0;
    };

    template <typename E, typename P>
    struct Element : ElementBase {
        Element(E elem, P& proc);
        
        Finalized<typename E::UniformsType> finalize(Constraints& constraints, Measured& measured, Atomized& atomized, Placed& placed);
        void encode(Finalized<typename E::UniformsType> finalized);

        E element;
        P& processor;
    };

}