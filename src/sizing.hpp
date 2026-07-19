#pragma once
#include <expected>
#include <optional>

namespace style {
    enum class SizeResolveFailure {
        Auto,
        IndefiniteBasis,
        FractionRequiresContext
    };

    enum class Unit {
        Px,
        Percent,
        Auto,
        Pt,
        Fr
    };

    struct Size {
        float value;
        Unit unit;

        Size():
            value{0},
            unit{Unit::Px}
        {};

        static Size px(float v)       { return {v, Unit::Px}; }
        static Size pt(float v)       { return {v, Unit::Pt}; }
        static Size percent(float v)  {
            if (v < 0.0)
                return {0.0, Unit::Percent};
            else if (v > 1.0)
                return {1.0, Unit::Percent};
            else
                return {v, Unit::Percent};
        }
        static Size autoSize()        { return {0.0f, Unit::Auto}; }
        static Size fr(float v)       { return {v, Unit::Fr}; }

        bool isAuto() const { return unit == Unit::Auto; }
        bool isFr() const { return unit == Unit::Fr; }

        std::expected<float, SizeResolveFailure> resolve(const Size& basis) const {
            switch (unit) {
                case Unit::Px:
                    return value;
                case Unit::Auto:
                    return std::unexpected(SizeResolveFailure::Auto);
                case Unit::Pt:
                    return value;
                case Unit::Fr:
                    return std::unexpected(SizeResolveFailure::FractionRequiresContext);
                case Unit::Percent:
                    if (basis.unit == Unit::Px || basis.unit == Unit::Pt) {
                        return value * basis.value;
                    }
                    return std::unexpected(SizeResolveFailure::IndefiniteBasis);
            }
        }

        float resolveOr(const Size& basis, float defaultVal = 0.0f) const {
            auto resolved = resolve(basis);
            return resolved.value_or(defaultVal);
        }

        private:
            constexpr Size(float v, Unit u) : value(v), unit(u) {}
    };
}
