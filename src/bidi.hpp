#pragma once

#include "result.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

namespace bidi {
    enum class BidiBaseDirection { Ltr, Rtl, AutoLtr, AutoRtl };
    enum class BidiError {
        AlgorithmCreationFailed,
        ParagraphCreationFailed,
        ScriptLocatorCreationFailed
    };

    std::error_code make_error_code(BidiError error);

    struct BidiRun {
        size_t byteStart{};
        size_t byteLength{};
        uint8_t level{};

        bool isRtl() const { return (level & 1u) != 0; }
    };

    class BidiParagraph {
    public:
        static Result<BidiParagraph> create(
            std::string_view utf8,
            BidiBaseDirection baseDirection
        );
        ~BidiParagraph();

        BidiParagraph(BidiParagraph&&) noexcept;
        BidiParagraph& operator=(BidiParagraph&&) noexcept;
        BidiParagraph(const BidiParagraph&) = delete;
        BidiParagraph& operator=(const BidiParagraph&) = delete;

        size_t byteLength() const;
        uint8_t baseLevel() const;
        std::span<const uint8_t> levels() const;
        std::optional<std::vector<BidiRun>> visualRuns(size_t byteStart, size_t byteLength) const;

    private:
        struct Impl;
        explicit BidiParagraph(std::unique_ptr<Impl> impl);
        std::unique_ptr<Impl> impl;
    };
}
