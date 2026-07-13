#include "bidi.hpp"

#include <SheenBidi/SheenBidi.h>
#include <string>
#include <utility>

namespace bidi {
    namespace {
        class BidiErrorCategory final : public std::error_category {
        public:
            const char* name() const noexcept override { return "bidi"; }

            std::string message(int value) const override {
                switch (static_cast<BidiError>(value)) {
                    case BidiError::AlgorithmCreationFailed:
                        return "failed to create bidi algorithm";
                    case BidiError::ParagraphCreationFailed:
                        return "failed to resolve bidi paragraph";
                    case BidiError::ScriptLocatorCreationFailed:
                        return "failed to create bidi script locator";
                }
                return "unknown bidi error";
            }
        };

        const BidiErrorCategory bidiErrorCategory;

        SBLevel toSheenLevel(BidiBaseDirection direction) {
            switch (direction) {
                case BidiBaseDirection::Ltr: return 0;
                case BidiBaseDirection::Rtl: return 1;
                case BidiBaseDirection::AutoLtr: return SBLevelDefaultLTR;
                case BidiBaseDirection::AutoRtl: return SBLevelDefaultRTL;
            }
            return 0;
        }
    }

    std::error_code make_error_code(BidiError error) {
        return {static_cast<int>(error), bidiErrorCategory};
    }

    struct BidiParagraph::Impl {
        ~Impl() {
            if (paragraph) SBParagraphRelease(paragraph);
            if (algorithm) SBAlgorithmRelease(algorithm);
        }

        std::string source;
        SBAlgorithmRef algorithm{};
        SBParagraphRef paragraph{};
        std::vector<uint8_t> resolvedLevels;
    };

    BidiParagraph::BidiParagraph(std::unique_ptr<Impl> impl): impl{std::move(impl)} {}

    Result<BidiParagraph> BidiParagraph::create(
        std::string_view utf8,
        BidiBaseDirection direction
    ) {
        auto impl = std::make_unique<Impl>();
        impl->source = utf8;
        SBCodepointSequence sequence{
            .stringEncoding = SBStringEncodingUTF8,
            .stringBuffer = impl->source.data(),
            .stringLength = impl->source.size()
        };
        impl->algorithm = SBAlgorithmCreate(&sequence);
        if (!impl->algorithm) {
            return std::unexpected{make_error_code(BidiError::AlgorithmCreationFailed)};
        }

        impl->paragraph = SBAlgorithmCreateParagraph(
            impl->algorithm, 0, impl->source.size(), toSheenLevel(direction));
        if (!impl->paragraph) {
            return std::unexpected{make_error_code(BidiError::ParagraphCreationFailed)};
        }

        const auto length = SBParagraphGetLength(impl->paragraph);
        const auto* data = SBParagraphGetLevelsPtr(impl->paragraph);
        impl->resolvedLevels.assign(data, data + length);
        return BidiParagraph{std::move(impl)};
    }
    BidiParagraph::~BidiParagraph() = default;
    BidiParagraph::BidiParagraph(BidiParagraph&&) noexcept = default;
    BidiParagraph& BidiParagraph::operator=(BidiParagraph&&) noexcept = default;

    size_t BidiParagraph::byteLength() const { return impl->source.size(); }
    uint8_t BidiParagraph::baseLevel() const { return SBParagraphGetBaseLevel(impl->paragraph); }
    std::span<const uint8_t> BidiParagraph::levels() const { return impl->resolvedLevels; }

    std::optional<std::vector<BidiRun>> BidiParagraph::visualRuns(size_t byteStart, size_t byteLength) const {
        if (byteStart > impl->source.size() || byteLength > impl->source.size() - byteStart) {
            return std::nullopt;
        }
        if (byteLength == 0) return {};

        SBLineRef line = SBParagraphCreateLine(impl->paragraph, byteStart, byteLength);
        if (!line) return std::nullopt;

        const auto count = SBLineGetRunCount(line);
        const auto* sourceRuns = SBLineGetRunsPtr(line);
        std::vector<BidiRun> runs;
        runs.reserve(count);
        for (SBUInteger i = 0; i < count; ++i) {
            runs.push_back({sourceRuns[i].offset, sourceRuns[i].length, sourceRuns[i].level});
        }
        SBLineRelease(line);
        return runs;
    }
}
