# Roadmap

## 1. Text Correctness

Goal: text should behave predictably enough to build real UI.

1. Fix hard newlines.
   - Make multiline text render reliably.
   - Route `Atom::placeOnNewLine` through inline text layout cleanly.

2. Add explicit line metrics.
   - `lineHeight(...)`
   - Default line height from font size.
   - Line boxes should use line metrics instead of tallest-glyph behavior.

3. Add explicit whitespace modes.
   - `whiteSpace: Normal | NoWrap | Pre | PreWrap`
   - `wordBreak: Normal | BreakAll`

4. Add text overflow.
   - Support clipping and a configurable overflow ending.

5. Add text alignment.
   - `textAlign: Start | Left | Center | Right`

6. Improve text shaping coverage.
   - Introduce shaped glyph runs.
   - Support glyph-id lookup instead of codepoint-only lookup.
   - Store per-glyph advances and offsets.
   - Add kerning.
   - Add ligatures.
   - Add complex script shaping.
   - Add bidirectional text handling.

### Text Backlog

- Consider multi-line clamping if a concrete UI use case requires it.
- Apply UAX #9 rule L1 after line wrapping so trailing whitespace uses the paragraph base level before visual fragment reordering.

## 2. Optimization

Goal: make the library fast and efficient under real app workloads.

Targets:
- Keep frame times under 16ms for at least 60fps rendering.
- Minimize cache misses.
- Improve vectorization.
- Use threading where it meaningfully reduces frame time or latency.
- Avoid unnecessarily wide instructions when narrower work is faster or more power efficient.
- Reduce idle CPU usage.
- Reduce RAM usage with many static images/SVGs loaded.
- Reduce tree overhead, especially where it causes cache misses or high CPU usage.

1. Audit dirty-bit precision.
   - Verify each builder mutation dirties the minimum required phases.
   - Separate topology changes from layout/style/text mutations.
   - Preserve current targeted paint invalidation for z-index/color-like changes.
   - Avoid root/subtree dirtying unless the mutation actually requires it.

2. Tighten constraints caching.
   - Audit `ConstraintsKey` coverage.
   - Avoid recomputing phases when incoming constraints and dirty bits are unchanged.

3. Validate render-order invalidation.
   - Rebuild render order only on topology or paint-order changes.
   - Ensure global z-index recalculation happens whenever paint order needs it.
   - Keep ancestor-aware ordering deterministic.

4. Unify hit-test paths.
   - Keep early-return hit testing and collecting hit testing consistent.
   - Avoid per-call child sorting where cached render order is sufficient.
   - Add subtree pruning by bounds if hit testing becomes a bottleneck.

5. Reduce layout allocations.
   - Reuse `lineFragments`, `lineBoxes`, atom offset buffers.
   - Reserve common vector sizes.
   - Avoid rebuilding temporary vectors in hot paths.

6. Optimize overflow scroll.
   - Recompute scroll content size only when descendant layout changes.
   - Keep scroll offset changes out of measure/layout when geometry is unchanged.

7. Improve text/glyph cache behavior.
   - Avoid repeated glyph-run work for unchanged text descriptors.
   - Keep glyph buffer growth predictable.
   - Reuse text placement buffers where possible.

8. Reduce idle CPU/GPU work.
   - Preserve the existing `RenderTree::update` early return for clean trees.
   - Avoid creating command buffers / render encoders / presents when no redraw is needed.
   - Audit MTK view scheduling so static UI does not continuously redraw.
   - Replace or tighten the resize watcher polling path if it burns idle CPU.

9. Add static asset resource caching.
   - Deduplicate images/SVGs by path or content key.
   - Share CPU-side decoded data and GPU-side resources where possible.
   - Track resource lifetime and release unused static assets.
   - Avoid loading the same static asset repeatedly across nodes.

## 3. Debugging Tools

Goal: make layout/render bugs visible.

1. Renderer stats panel.
   - Frame time.
   - Per-phase timing.
   - Nodes measured/atomized/laid out/placed/finalized.
   - Render order cache hit/miss.
   - Hit-test count.

2. Dirty diagnostics.
   - Show which node dirtied.
   - Show which phases dirtied.
   - Track dirty reasons at mutation sites.
   - Explain cache misses when a phase recomputes.

3. Selected node overlay.
   - Draw content box.
   - Draw padding box.
   - Draw margin box.
   - Show node id/type label near box.

4. Hit stack panel.
   - List all hit nodes, not just `hit: n / total`.
   - Selected row highlighted.
   - Shift+Up/Down moves selection.

5. Dirty phase panel.
   - Per selected node: measure/atomize/layout/postLayout/place/finalize/paint.
   - Show last dirtied reason.

6. Render order panel.
   - Selected node global z.
   - Previous/next render neighbors.
   - Parent/child paint relationship.

7. Tree view.
   - Collapsible tree.
   - Selected node highlighted.
   - Show `Element#id`.
   - Search by id.

8. Event view.
   - Recent events.
   - Focused node.
   - Hovered node.
   - Mouse down target.
   - Listeners attached to selected node.

## 4. Layout / UI Features

Goal: expand the engine toward practical app UI.

1. Better sizing.
   - `fit-content`
   - `min-content`
   - `max-content`
   - `aspectRatio(...)`

2. Better positioning.
   - `position: sticky`
   - `inset(...)`
   - `pointerEvents: Auto | None`

3. Better visual styling.
   - Opacity.
   - Box shadow.
   - Background image.
   - Linear gradients.
   - Border styles: solid/dashed.
   - Per-corner radius.

4. Transforms.
   - Translate.
   - Scale.
   - Rotate.
   - Transform origin.
   - Transformed hit testing.

5. Interaction primitives.
   - Hover state.
   - Active/pressed state.
   - Focus-visible state.
   - Tab navigation.
   - Keyboard shortcut normalization.

6. Layout transitions.
   - Animate position changes.
   - Animate size changes.
   - Animate opacity.
   - Animate transform.

7. Portals / overlay layer.
   - Tooltip.
   - Popover.
   - Modal.
   - Context menu.
   - Ensure overlay z-order is not ad hoc.

## 5. Platform / Backend Split

Goal: prepare the engine for Metal and Vulkan backends without mixing rendering with platform services.

1. Define a render backend boundary.
   - Device.
   - Swapchain/drawable.
   - Command buffer.
   - Buffers.
   - Textures.
   - Pipelines.
   - Shader/library loading.

2. Define a platform backend boundary.
   - Window creation.
   - Event pump.
   - Keyboard/mouse input.
   - Clipboard.
   - Cursor.
   - File dialogs.
   - Font discovery.
   - Accessibility bridge.

3. Move AppKit-specific code behind platform services.
   - Keep Objective-C runtime hooks out of core renderer code.
   - Convert native events into backend-neutral runtime events.

4. Move Metal-specific code behind render services.
   - Keep Metal resource types out of high-level UI code.
   - Preserve backend-neutral element/layout APIs.

5. Add Vulkan backend prototype.
   - Buffer abstraction.
   - Pipeline abstraction.
   - Shader translation/build path.
   - Swapchain presentation.

## 6. Resize / Multithreaded Animation

Goal: resize should feel browser-smooth.

1. Separate live tree mutation from render snapshots.
   - UI tree mutates.
   - Renderer consumes immutable snapshot.

2. Double-buffer snapshots.
   - Current snapshot displayed.
   - Next snapshot computed.

3. Add resize scheduler.
   - Resize event updates target frame size.
   - Renderer keeps presenting current valid frame.
   - Layout thread computes next snapshot.

4. Add interpolation.
   - Keep previous layout boxes.
   - Interpolate to next layout boxes.
   - Use interpolation only for resize/layout transitions.

5. Move layout prep off the render thread.
   - Measure/layout/postLayout on worker.
   - Render thread only encodes stable snapshot.

6. Add synchronization.
   - Atomic snapshot swap.
   - No partial tree reads.
   - No mutation while render snapshot is active.

7. Add fallback behavior.
   - If next snapshot is late, keep presenting previous.
   - If resize ends, snap to final computed layout.

## Order

```text
1. Text correctness
2. Optimization
3. Debugging tools
4. Layout/UI feature expansion
5. Platform/backend split
6. Resize + multithreaded animation
```
