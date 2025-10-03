//
//  fragment.hpp
//  gui
//
//  Created by Taanish Reja on 10/2/25.
//

#pragma once

// drawable fragment
struct Fragment {
    Fragment(int fx, int fy, int offset, int length);
    
    int fx;
    int fy;
    int offset;
    int length;
};
