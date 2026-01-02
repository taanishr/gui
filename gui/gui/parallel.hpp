#include <iterator>
#include <dispatch/dispatch.h>

namespace Parallel {
    template<typename Iter, typename Func>
    void for_each(Iter begin, Iter end, Func&& func) {
        #ifdef __APPLE__
                dispatch_apply(std::distance(begin, end), 
                            dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0),
                    ^(size_t i) {
                        func(*(begin + i));
                    }
                );
        #else
                // serial fallback (later will add portability)
                std::for_each(begin, end, std::forward<Func>(func));
        #endif
    }
}