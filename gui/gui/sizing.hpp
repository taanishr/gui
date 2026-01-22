#pragma once

namespace NewArch {
    enum class Unit {
        Px,
        Percent,
    };

    struct Size {
        float value;
        Unit unit;

        static Size px(float v)       { return {v, Unit::Px}; }
        static Size percent(float v)  { 
            if (v < 0.0)
                return {0.0, Unit::Percent};
            else if (v > 1.0)
                return {1.0, Unit::Percent};
            else
                return {v, Unit::Percent};
        }

        float resolve(float referenceSize) const {
            switch (unit) {
                case Unit::Px:
                    return value;
                case Unit::Percent:
                    return value * referenceSize;
            }
        }
    };
}