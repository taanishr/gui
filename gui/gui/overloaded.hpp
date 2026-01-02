//
//  overloadaed.hpp
//  gui
//
//  Created by Taanish Reja on 9/16/25.
//


template<class... Ts>
struct Overloaded : Ts... {
    using Ts::operator()...;
};
