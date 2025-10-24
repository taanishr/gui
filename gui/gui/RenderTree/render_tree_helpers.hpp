//
//  render_tree_helpers.hpp
//  gui
//
//  Created by Taanish Reja on 10/24/25.
//

struct Atom {
    float width;
    float height;
}

struct FragmentTemplate {
    std::vector<Atom> atoms; // individual atoms
    float width; // float of fragment
    float height; // height of fragment
}

struct Constraints {
    float maxWidth;
    float maxHeight;
    float explicitWidth;
    float explicitHeight;
}
