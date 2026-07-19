#pragma once

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <source_location>
#include <string>
#include <unordered_map>

#ifndef GUI_ENABLE_INSTRUMENTATION
#define GUI_ENABLE_INSTRUMENTATION 0
#endif

namespace instrumentation {
    inline constexpr bool enabled = GUI_ENABLE_INSTRUMENTATION;
    inline constexpr std::size_t FrameHistoryCapacity = 8;
    inline constexpr std::size_t MutationHistoryCapacity = 64;

    enum class Phase : uint8_t {
        Update,
        Measure,
        Atomize,
        PreLayout,
        Layout,
        PostLayout,
        Place,
        Finalize,
        Render,
        Count
    };

    enum class RecomputeReason : uint8_t {
        None,
        Dirty,
        MissingConstraintsKey,
        ConstraintsChanged,
        AncestorRecomputed
    };

    enum class FrameReason : uint8_t {
        None = 0,
        Mutation = 1 << 0,
        FrameInfoChanged = 1 << 1,
        PendingBufferWrites = 1 << 2
    };

    enum class DirtyPropagation : uint8_t {
        None = 0,
        Ancestors = 1 << 0,
        Descendants = 1 << 1
    };

    enum class RenderOrderReason : uint8_t {
        None = 0,
        Initial = 1 << 0,
        FullTreeDirty = 1 << 1,
        PaintOrderChanged = 1 << 2,
        FrameInfoChanged = 1 << 3,
        EmptyCache = 1 << 4
    };

    struct PhaseDiagnostics {
        std::chrono::nanoseconds elapsed{};
        uint64_t recomputedNodes{};
    };

    struct MutationDiagnostics {
        uint64_t sourceNodeId{};
        uint32_t requestedDirtyBits{};
        uint32_t effectiveSelfDirtyBits{};
        DirtyPropagation propagation{};
        std::string file;
        std::string function;
        uint32_t line{};
        uint32_t column{};
    };

    struct CacheDiagnostics {
        uint64_t hits{};
        uint64_t misses{};
        std::chrono::nanoseconds rebuildTime{};
        uint32_t lastRebuildReasons{};
    };

    struct RenderDiagnostics {
        uint64_t nodesEncoded{};
        uint64_t drawCalls{};
        uint64_t atomsRendered{};
        uint64_t bufferWrites{};
        uint64_t bufferBytes{};
    };

    struct HitTestDiagnostics {
        uint64_t calls{};
        uint64_t nodesExamined{};
        uint64_t hits{};
        std::chrono::nanoseconds elapsed{};
    };

    struct FrameDiagnostics {
        uint64_t frameIndex{};
        uint32_t reasons{};
        std::chrono::nanoseconds elapsed{};
        std::array<PhaseDiagnostics, static_cast<std::size_t>(Phase::Count)> phases{};
        CacheDiagnostics renderOrderCache;
        CacheDiagnostics speculativeLayoutCache;
        RenderDiagnostics render;
        HitTestDiagnostics hitTests;
        std::deque<MutationDiagnostics> mutations;
    };

    struct NodeDiagnostics {
        std::array<RecomputeReason, static_cast<std::size_t>(Phase::Count)> lastRecomputeReasons{};
        std::array<uint64_t, static_cast<std::size_t>(Phase::Count)> recomputeCounts{};
        MutationDiagnostics lastMutation;
    };

    struct SchedulingDiagnostics {
        uint64_t drawRequests{};
        uint64_t skippedDraws{};
        uint32_t lastFrameReasons{};
    };

    class Diagnostics {
    public:
        void beginFrame(uint64_t frameIndex);
        void endFrame(std::chrono::nanoseconds elapsed);
        void addPhaseTime(Phase phase, std::chrono::nanoseconds elapsed);
        void recordRecompute(uint64_t nodeId, Phase phase, RecomputeReason reason);
        void recordFrameDecision(uint32_t reasons);
        void recordMutation(
            uint64_t sourceNodeId,
            uint32_t requestedDirtyBits,
            uint32_t effectiveSelfDirtyBits,
            DirtyPropagation propagation,
            std::source_location source
        );
        void recordRenderOrderInvalidation(uint32_t reason);
        void recordRenderOrderCache(
            bool hit,
            uint32_t immediateReason,
            std::chrono::nanoseconds rebuildTime
        );
        void recordSpeculativeLayoutCache(bool hit);
        void recordRenderWork(uint64_t nodes, uint64_t drawCalls, uint64_t atoms);
        void recordBufferWrite(uint64_t bytes);
        void recordHitTest(uint64_t nodesExamined, uint64_t hits, std::chrono::nanoseconds elapsed);
        void removeNode(uint64_t nodeId);

        const FrameDiagnostics& latestFrame() const;
        const std::deque<FrameDiagnostics>& frames() const;
        const SchedulingDiagnostics& scheduling() const;
        const std::unordered_map<uint64_t, NodeDiagnostics>& nodes() const;

    private:
        FrameDiagnostics& targetFrame();

        bool frameActive{};
        uint32_t pendingFrameReasons{};
        uint32_t renderOrderReasons{};
        FrameDiagnostics pendingFrame;
        FrameDiagnostics currentFrame;
        FrameDiagnostics emptyFrame;
        std::deque<FrameDiagnostics> frameHistory;
        SchedulingDiagnostics schedulingDiagnostics;
        std::unordered_map<uint64_t, NodeDiagnostics> nodeDiagnostics;
    };

    Diagnostics& getDiagnostics();

    inline void recordRecompute(uint64_t nodeId, Phase phase, RecomputeReason reason) {
        if constexpr (enabled) getDiagnostics().recordRecompute(nodeId, phase, reason);
    }

    inline void recordFrameDecision(uint32_t reasons) {
        if constexpr (enabled) getDiagnostics().recordFrameDecision(reasons);
    }

    inline void recordMutation(
        uint64_t sourceNodeId,
        uint32_t requestedDirtyBits,
        uint32_t effectiveSelfDirtyBits,
        DirtyPropagation propagation,
        std::source_location source = std::source_location::current()
    ) {
        if constexpr (enabled) {
            getDiagnostics().recordMutation(
                sourceNodeId,
                requestedDirtyBits,
                effectiveSelfDirtyBits,
                propagation,
                source
            );
        }
    }

    inline void recordRenderOrderInvalidation(uint32_t reason) {
        if constexpr (enabled) getDiagnostics().recordRenderOrderInvalidation(reason);
    }

    inline void recordRenderOrderCache(
        bool hit,
        uint32_t reasons = 0,
        std::chrono::nanoseconds rebuildTime = {}
    ) {
        if constexpr (enabled) getDiagnostics().recordRenderOrderCache(hit, reasons, rebuildTime);
    }

    inline void recordSpeculativeLayoutCache(bool hit) {
        if constexpr (enabled) getDiagnostics().recordSpeculativeLayoutCache(hit);
    }

    inline void recordRenderWork(uint64_t nodes, uint64_t drawCalls, uint64_t atoms) {
        if constexpr (enabled) getDiagnostics().recordRenderWork(nodes, drawCalls, atoms);
    }

    inline void recordBufferWrite(uint64_t bytes) {
        if constexpr (enabled) getDiagnostics().recordBufferWrite(bytes);
    }

    inline void recordHitTest(
        uint64_t nodesExamined,
        uint64_t hits,
        std::chrono::nanoseconds elapsed
    ) {
        if constexpr (enabled) getDiagnostics().recordHitTest(nodesExamined, hits, elapsed);
    }

    inline void removeNode(uint64_t nodeId) {
        if constexpr (enabled) getDiagnostics().removeNode(nodeId);
    }

    template<bool Enabled>
    class BasicPhaseTimer;

    template<>
    class BasicPhaseTimer<false> {
    public:
        explicit constexpr BasicPhaseTimer(Phase) noexcept {}
    };

    template<>
    class BasicPhaseTimer<true> {
    public:
        explicit BasicPhaseTimer(Phase phase);
        ~BasicPhaseTimer();

        BasicPhaseTimer(const BasicPhaseTimer&) = delete;
        BasicPhaseTimer& operator=(const BasicPhaseTimer&) = delete;

    private:
        Phase phase;
        std::chrono::steady_clock::time_point startedAt;
    };

    template<bool Enabled>
    class BasicFrameTimer;

    template<>
    class BasicFrameTimer<false> {
    public:
        explicit constexpr BasicFrameTimer(uint64_t) noexcept {}
    };

    template<>
    class BasicFrameTimer<true> {
    public:
        explicit BasicFrameTimer(uint64_t frameIndex);
        ~BasicFrameTimer();

        BasicFrameTimer(const BasicFrameTimer&) = delete;
        BasicFrameTimer& operator=(const BasicFrameTimer&) = delete;

    private:
        std::chrono::steady_clock::time_point startedAt;
    };

    using PhaseTimer = BasicPhaseTimer<enabled>;
    using FrameTimer = BasicFrameTimer<enabled>;
}
