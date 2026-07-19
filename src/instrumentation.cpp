#include "instrumentation.hpp"

#include <mutex>
#include <optional>

namespace instrumentation {
    namespace {
        constexpr std::size_t phaseIndex(Phase phase) {
            return static_cast<std::size_t>(phase);
        }
    }

    Diagnostics& getDiagnostics() {
        static std::once_flag initFlag;
        static std::optional<Diagnostics> diagnostics;

        std::call_once(initFlag, [&] {
            diagnostics.emplace();
        });

        return *diagnostics;
    }

    FrameDiagnostics& Diagnostics::targetFrame() {
        return frameActive ? currentFrame : pendingFrame;
    }

    void Diagnostics::beginFrame(uint64_t frameIndex) {
        currentFrame = std::move(pendingFrame);
        pendingFrame = {};
        currentFrame.frameIndex = frameIndex;
        currentFrame.reasons = pendingFrameReasons;
        pendingFrameReasons = 0;
        frameActive = true;
    }

    void Diagnostics::endFrame(std::chrono::nanoseconds elapsed) {
        currentFrame.elapsed = elapsed;
        frameHistory.push_back(std::move(currentFrame));
        if (frameHistory.size() > FrameHistoryCapacity) {
            frameHistory.pop_front();
        }
        currentFrame = {};
        frameActive = false;
    }

    void Diagnostics::addPhaseTime(Phase phase, std::chrono::nanoseconds elapsed) {
        currentFrame.phases[phaseIndex(phase)].elapsed += elapsed;
    }

    void Diagnostics::recordRecompute(uint64_t nodeId, Phase phase, RecomputeReason reason) {
        auto index = phaseIndex(phase);
        targetFrame().phases[index].recomputedNodes++;

        auto& node = nodeDiagnostics[nodeId];
        node.lastRecomputeReasons[index] = reason;
        node.recomputeCounts[index]++;
    }

    void Diagnostics::recordFrameDecision(uint32_t reasons) {
        schedulingDiagnostics.drawRequests++;
        schedulingDiagnostics.lastFrameReasons = reasons;
        if (reasons == 0) {
            schedulingDiagnostics.skippedDraws++;
        } else {
            pendingFrameReasons = reasons;
        }
    }

    void Diagnostics::recordMutation(
        uint64_t sourceNodeId,
        uint32_t requestedDirtyBits,
        uint32_t effectiveSelfDirtyBits,
        DirtyPropagation propagation,
        std::source_location source
    ) {
        MutationDiagnostics mutation {
            .sourceNodeId = sourceNodeId,
            .requestedDirtyBits = requestedDirtyBits,
            .effectiveSelfDirtyBits = effectiveSelfDirtyBits,
            .propagation = propagation,
            .file = source.file_name(),
            .function = source.function_name(),
            .line = source.line(),
            .column = source.column()
        };

        nodeDiagnostics[sourceNodeId].lastMutation = mutation;

        auto& mutations = targetFrame().mutations;
        mutations.push_back(std::move(mutation));
        if (mutations.size() > MutationHistoryCapacity) {
            mutations.pop_front();
        }
    }

    void Diagnostics::recordRenderOrderInvalidation(uint32_t reason) {
        renderOrderReasons |= reason;
    }

    void Diagnostics::recordRenderOrderCache(
        bool hit,
        uint32_t rebuildReasons,
        std::chrono::nanoseconds rebuildTime
    ) {
        auto& cache = targetFrame().renderOrderCache;
        if (hit) {
            cache.hits++;
            return;
        }
        cache.misses++;
        cache.rebuildTime += rebuildTime;
        cache.lastRebuildReasons = renderOrderReasons | rebuildReasons;
        renderOrderReasons = 0;
    }

    void Diagnostics::recordSpeculativeLayoutCache(bool hit) {
        auto& cache = targetFrame().speculativeLayoutCache;
        hit ? cache.hits++ : cache.misses++;
    }

    void Diagnostics::recordRenderWork(uint64_t nodes, uint64_t drawCalls, uint64_t atoms) {
        auto& render = targetFrame().render;
        render.nodesEncoded += nodes;
        render.drawCalls += drawCalls;
        render.atomsRendered += atoms;
    }

    void Diagnostics::recordBufferWrite(uint64_t bytes) {
        auto& render = targetFrame().render;
        render.bufferWrites++;
        render.bufferBytes += bytes;
    }

    void Diagnostics::recordHitTest(
        uint64_t nodesExamined,
        uint64_t hits,
        std::chrono::nanoseconds elapsed
    ) {
        auto& hitTests = targetFrame().hitTests;
        hitTests.calls++;
        hitTests.nodesExamined += nodesExamined;
        hitTests.hits += hits;
        hitTests.elapsed += elapsed;
    }

    void Diagnostics::removeNode(uint64_t nodeId) {
        nodeDiagnostics.erase(nodeId);
    }

    const FrameDiagnostics& Diagnostics::latestFrame() const {
        return frameHistory.empty() ? emptyFrame : frameHistory.back();
    }

    const std::deque<FrameDiagnostics>& Diagnostics::frames() const {
        return frameHistory;
    }

    const SchedulingDiagnostics& Diagnostics::scheduling() const {
        return schedulingDiagnostics;
    }

    const std::unordered_map<uint64_t, NodeDiagnostics>& Diagnostics::nodes() const {
        return nodeDiagnostics;
    }

    BasicPhaseTimer<true>::BasicPhaseTimer(Phase phase):
        phase{phase},
        startedAt{std::chrono::steady_clock::now()}
    {}

    BasicPhaseTimer<true>::~BasicPhaseTimer() {
        getDiagnostics().addPhaseTime(
            phase,
            std::chrono::steady_clock::now() - startedAt
        );
    }

    BasicFrameTimer<true>::BasicFrameTimer(uint64_t frameIndex):
        startedAt{std::chrono::steady_clock::now()}
    {
        getDiagnostics().beginFrame(frameIndex);
    }

    BasicFrameTimer<true>::~BasicFrameTimer() {
        getDiagnostics().endFrame(
            std::chrono::steady_clock::now() - startedAt
        );
    }
}
