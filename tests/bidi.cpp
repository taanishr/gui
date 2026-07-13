#include "bidi.hpp"
#include "text_bidi.hpp"

#include <cassert>

int main() {
    {
        auto result = bidi::BidiParagraph::create(
            "plain text", bidi::BidiBaseDirection::Ltr);
        assert(result.has_value());
        auto& paragraph = *result;
        auto runs = paragraph.visualRuns(0, paragraph.byteLength());
        assert(paragraph.baseLevel() == 0);
        assert(runs.has_value() && runs->size() == 1);
        assert(!runs->front().isRtl());
    }
    {
        constexpr std::string_view mixed = "abc \u05d0\u05d1\u05d2 123";
        auto result = bidi::BidiParagraph::create(mixed, bidi::BidiBaseDirection::Ltr);
        assert(result.has_value());
        auto& paragraph = *result;
        auto runs = paragraph.visualRuns(0, paragraph.byteLength());
        assert(runs.has_value() && runs->size() >= 3);
        size_t coveredBytes = 0;
        for (const auto& run : *runs) coveredBytes += run.byteLength;
        assert(coveredBytes == paragraph.byteLength());
    }
    {
        constexpr std::string_view rtl = "\u05d0\u05d1\u05d2";
        auto result = bidi::BidiParagraph::create(rtl, bidi::BidiBaseDirection::Rtl);
        assert(result.has_value());
        auto& paragraph = *result;
        assert(paragraph.baseLevel() == 1);
        assert(paragraph.visualRuns(0, paragraph.byteLength())->front().isRtl());
    }
    {
        auto result = bidi::TextBidiContext::create(
            "abc \u05d0\u05d1\u05d2\n\u05d0\u05d1\u05d2 xyz",
            bidi::BidiBaseDirection::Ltr
        );
        assert(result.has_value());
        auto& context = *result;
        auto runs = context.runs();
        assert(!runs.empty());
        size_t coveredBytes = 0;
        for (const auto& run : runs) coveredBytes += run.byteLength;
        assert(coveredBytes == context.text().size());
    }
}
