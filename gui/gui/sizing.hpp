#pragma once
#include <optional>

namespace NewArch {
    enum class Unit {
        Px,
        Percent,
        Auto,
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
        static Size autoSize()        { return {0.0f, Unit::Auto}; }

        bool isAuto() const { return unit == Unit::Auto; }

        std::optional<float> resolve(float referenceSize) const {
            switch (unit) {
                case Unit::Px:
                    return value;
                case Unit::Percent:
                    return value * referenceSize;
                case Unit::Auto:
                    return std::nullopt;
            }
        }

        float resolveOr(float referenceSize, float defaultVal = 0.0f) const {
            auto resolved = resolve(referenceSize);
            return resolved.value_or(defaultVal);
        }
    };
}