#include "text_bidi.hpp"

#include "utf8.hpp"
#include <SheenBidi/SheenBidi.h>
#include <algorithm>

namespace bidi {
    TextBidiContext::TextBidiContext(std::string text, BidiBaseDirection direction):
        source{std::move(text)},
        direction{direction}
    {}

    Result<TextBidiContext> TextBidiContext::create(
        std::string text,
        BidiBaseDirection direction
    ) {
        TextBidiContext context{std::move(text), direction};
        size_t paragraphStart = 0;
        for (size_t offset = 0; offset < context.source.size();) {
            const auto codepoint = utf8::at(context.source, offset);
            offset += codepoint.byteLength;
            if (codepoint.value != U'\r' && codepoint.value != U'\n') continue;

            // in some files, \r may preceed a \n for a single combined newline
            // this is common in files produced by windows
            if (codepoint.value == U'\r' && offset < context.source.size()) {
                const auto next = utf8::at(context.source, offset);
                if (next.value == U'\n') offset += next.byteLength;
            }

            const size_t length = offset - paragraphStart;
            auto resolved = BidiParagraph::create(
                std::string_view{context.source}.substr(paragraphStart, length), direction);
            if (!resolved) return std::unexpected{resolved.error()};
            context.paragraphs.push_back({
                .byteStart = paragraphStart,
                .byteLength = length,
                .resolved = std::make_unique<BidiParagraph>(std::move(*resolved))
            });
            paragraphStart = offset;
        }

        if (paragraphStart < context.source.size() || context.paragraphs.empty()) {
            const size_t length = context.source.size() - paragraphStart;
            auto resolved = BidiParagraph::create(
                std::string_view{context.source}.substr(paragraphStart, length), direction);
            if (!resolved) return std::unexpected{resolved.error()};
            context.paragraphs.push_back({
                .byteStart = paragraphStart,
                .byteLength = length,
                .resolved = std::make_unique<BidiParagraph>(std::move(*resolved))
            });
        }

        SBCodepointSequence sequence{
            .stringEncoding = SBStringEncodingUTF8,
            .stringBuffer = context.source.data(),
            .stringLength = context.source.size()
        };
        SBScriptLocatorRef locator = SBScriptLocatorCreate();
        if (!locator) {
            return std::unexpected{
                make_error_code(BidiError::ScriptLocatorCreationFailed)};
        }
        SBScriptLocatorLoadCodepoints(locator, &sequence);
        size_t paragraphIndex = 0;
        while (SBScriptLocatorMoveNext(locator)) {
            const auto* script = SBScriptLocatorGetAgent(locator);
            const size_t scriptEnd = script->offset + script->length;
            const uint32_t scriptTag = SBScriptGetUnicodeTag(script->script);
            size_t offset = script->offset;

            while (offset < scriptEnd) {
                while (paragraphIndex + 1 < context.paragraphs.size() &&
                       offset >= context.paragraphs[paragraphIndex].byteStart +
                           context.paragraphs[paragraphIndex].byteLength) {
                    ++paragraphIndex;
                }

                const auto& paragraph = context.paragraphs[paragraphIndex];
                const size_t paragraphEnd = paragraph.byteStart + paragraph.byteLength;
                const auto levels = paragraph.resolved->levels();
                const uint8_t level = levels[offset - paragraph.byteStart];

                size_t runEnd = offset + utf8::at(context.source, offset).byteLength;
                while (runEnd < scriptEnd && runEnd < paragraphEnd &&
                       levels[runEnd - paragraph.byteStart] == level) {
                    runEnd += utf8::at(context.source, runEnd).byteLength;
                }

                context.shapingRuns.push_back({
                    .byteStart = offset,
                    .byteLength = runEnd - offset,
                    .level = level,
                    .scriptTag = scriptTag
                });
                offset = runEnd;
            }
        }
        SBScriptLocatorRelease(locator);
        return context;
    }

    std::optional<std::vector<BidiRun>> TextBidiContext::visualRuns(
        size_t byteStart,
        size_t byteLength
    ) const {
        const size_t byteEnd = byteStart + byteLength;
        for (const auto& paragraph : paragraphs) {
            const size_t paragraphEnd = paragraph.byteStart + paragraph.byteLength;
            if (byteStart < paragraph.byteStart || byteEnd > paragraphEnd) continue;
            auto runs = paragraph.resolved->visualRuns(
                byteStart - paragraph.byteStart,
                byteLength
            );
            if (!runs.has_value()) return std::nullopt;
            for (auto& run : *runs) run.byteStart += paragraph.byteStart;
            return runs;
        }
        return std::nullopt;
    }

}
