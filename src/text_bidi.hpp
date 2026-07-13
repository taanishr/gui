#pragma once

#include "bidi.hpp"
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace bidi {
    struct TextShapingRun {
        size_t byteStart{};
        size_t byteLength{};
        uint8_t level{};
        uint32_t scriptTag{};

        bool isRtl() const { return (level & 1u) != 0; }
    };

    class TextBidiContext {
    public:
        // The source must already reflect white-space processing for paragraph
        // separators while preserving byte offsets into the original text.
        static Result<TextBidiContext> create(
            std::string text,
            BidiBaseDirection direction
        );

        std::string_view text() const { return source; }
        std::span<const TextShapingRun> runs() const { return shapingRuns; }
        std::optional<std::vector<BidiRun>> visualRuns(size_t byteStart, size_t byteLength) const;

    private:
        TextBidiContext(std::string text, BidiBaseDirection direction);

        struct Paragraph {
            size_t byteStart{};
            size_t byteLength{};
            std::unique_ptr<BidiParagraph> resolved;
        };

        std::string source;
        BidiBaseDirection direction;
        std::vector<Paragraph> paragraphs;
        std::vector<TextShapingRun> shapingRuns;
    };

    struct TextBidiInput {
        std::shared_ptr<const TextBidiContext> context;
        size_t paragraphByteStart{};
        size_t byteLength{};
        std::vector<TextShapingRun> runs;
    };
}
