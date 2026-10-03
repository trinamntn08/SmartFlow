# Roadmap and current state

Updated: 2026-10-03. Native C++17/Qt6 on Windows is the primary implementation.
The application domain and audience remain open; included 3D support is required.

## Available now

SmartFlow is a working desktop development preview with graph editing, typed
connections, inline numeric node controls, semantic undo/redo, background
execution, cancellation and result viewers. Project files preserve unsupported
extension content and other graphs; unsupported graphs report diagnostics and
cannot execute. Open/Save/Save As and workspace restoration are implemented.
Widget regions have selectors, horizontal/vertical splitting, closing, swapping
and adjustable dividers; their layout is saved with the project. See
[workspace widgets](WORKSPACE_WIDGETS.md).

Native **Build/Use** switching presents a collapsed component as a tool with
exposed numeric controls and a selected output viewer. Both modes share commands,
undo/redo and background execution. Mode/tool/output selection and viewer state
save separately from Build layout and navigation. See [Use mode](USE_MODE.md)
and the [checkpoint](agents/handoffs/2026-10-03-native-use-mode.md).

The main app includes Cube, Transform, Material, Scene and Merge with a primitive
3D viewer. The independent data app has Sample table, Filter rows, Summary and a
table viewer; it does not link the scene extension or scene-math libraries.

Components support extraction, saved catalogs, collapsed snapshot instances,
exposed ports/controls, output selection/pinning, optional expanded insertion,
standalone import/export and undoable library removal. They are native source-level
features, not a production runtime or dynamically loaded plugin system.
Definitions can be edited as fresh-ID copies in isolated draft workspaces.
Compatible collapsed instances can explicitly adopt a chosen definition, with
preserved control values/connections and undo of the exact previous snapshot.

VS Code CMake Tools configure/build/test presets and launch configurations are
ready for manual use. A CMake install folder bundles both executables, Qt/MSVC
runtime dependencies and the scene, data and component examples.
The refreshed Windows folder includes N7, E1-E4, native Use mode, modular scene
interaction and inline node controls, with thirteen installed startup/render
checks and matching executable hashes. See
[the first test version and result sheet](TEST_VERSION.md).

## Verification and desktop acceptance

The latest implementation/configuration checks passed: native Release build,
32 CTest entries, Windows startup smoke checks and `npm.cmd run check` (formatting,
TypeScript, workspace behavior tests and web production build).

**Manual testing reported positive so far.** On 2026-10-03 the user confirmed:
"already manually tested, it's ok so far". No issues were reported. This feedback
supports moving on to the next product experiment; repeating initial manual
testing is not the immediate priority. Individual checklist coverage, executable
version and test environment were not specified, so full N5 checklist acceptance
and clean-machine compatibility are not established by this summary. See the
[feedback checkpoint](agents/handoffs/2026-10-03-manual-test-feedback.md).
Automated native computer-use remains deferred after an unavailable pipe.

Use [desktop testing](DESKTOP_TESTING.md) to record those results and
[VS Code setup](NATIVE_DEVELOPMENT.md#run-from-vs-code-with-cmake-tools) to run the
development build. The browser starter is deferred and cannot satisfy desktop
acceptance.

## Completed checkpoints

These rows describe delivered behavior; dated handoffs retain each step's original
verification and limitations. Later checkpoints supersede earlier pending items.

| Checkpoint          | Delivered behavior                                                                 | Record                                                        |
| ------------------- | ---------------------------------------------------------------------------------- | ------------------------------------------------------------- |
| TypeScript contract | Schema-v1 persistence and provisional SDK contracts                                | [contract](agents/handoffs/2026-09-27-contract-checkpoint.md) |
| N1                  | Copied QtNodes canvas and native shell                                             | [N1](agents/handoffs/2026-09-27-native-checkpoint-n1.md)      |
| N2                  | Audited pipeline foundation and background execution adapter                       | [N2](agents/handoffs/2026-09-28-native-checkpoint-n2.md)      |
| N3                  | Workspace, inspector and command-based edits                                       | [N3](agents/handoffs/2026-09-30-native-checkpoint-n3.md)      |
| N4                  | Included primitive scene extension and viewer                                      | [N4](agents/handoffs/2026-09-30-native-checkpoint-n4.md)      |
| N5a                 | Atomic schema-v1 project file codec                                                | [N5a](agents/handoffs/2026-09-30-native-checkpoint-n5a.md)    |
| N5b                 | Retained-document execution projection                                             | [N5b](agents/handoffs/2026-09-30-native-checkpoint-n5b.md)    |
| N5c                 | Structural retained project commands                                               | [N5c](agents/handoffs/2026-09-30-native-checkpoint-n5c.md)    |
| N5d                 | Retained semantic undo/redo                                                        | [N5d](agents/handoffs/2026-10-01-native-checkpoint-n5d.md)    |
| N5e                 | Canvas/inspector integration and unavailable placeholders                          | [N5e](agents/handoffs/2026-10-01-native-checkpoint-n5e.md)    |
| N5f                 | Open/Save/Save As and dirty-state tracking                                         | [N5f](agents/handoffs/2026-10-01-native-checkpoint-n5f.md)    |
| N5g                 | Layout/navigation, selection, pinned output and viewer persistence                 | [N5g](agents/handoffs/2026-10-01-native-checkpoint-n5g.md)    |
| N5h                 | Independent native table workflow                                                  | [N5h](agents/handoffs/2026-10-01-native-checkpoint-n5h.md)    |
| N5i                 | Shipped examples and fresh-process project launch checks                           | [N5i](agents/handoffs/2026-10-01-native-checkpoint-n5i.md)    |
| N6a                 | Reusable definitions and atomic expanded insertion                                 | [N6a](agents/handoffs/2026-10-01-native-checkpoint-n6a.md)    |
| N6b                 | Selection extraction and catalog commands                                          | [N6b](agents/handoffs/2026-10-02-native-checkpoint-n6b.md)    |
| N6c                 | Native authoring/library dialogs                                                   | [N6c](agents/handoffs/2026-10-02-native-checkpoint-n6c.md)    |
| N6d                 | Collapsed snapshots, exposed controls/ports and grouped results                    | [N6d](agents/handoffs/2026-10-02-native-checkpoint-n6d.md)    |
| N6e                 | Standalone component import/export                                                 | [N6e](agents/handoffs/2026-10-02-native-checkpoint-n6e.md)    |
| N6f                 | Undoable catalog-entry removal, preserving instances                               | [N6f](agents/handoffs/2026-10-02-native-checkpoint-n6f.md)    |
| N6g                 | Isolated component editing saved as immutable copies                               | [N6g](agents/handoffs/2026-10-02-native-checkpoint-n6g.md)    |
| N6h                 | Explicit compatible snapshot update for one collapsed instance                     | [N6h](agents/handoffs/2026-10-02-native-checkpoint-n6h.md)    |
| N7                  | Selectable widget regions, split/swap/close and saved divider layout               | [N7](agents/handoffs/2026-10-02-native-checkpoint-n7.md)      |
| IDE setup           | VS Code CMake Tools presets and native launch configurations                       | [IDE](agents/handoffs/2026-10-02-vscode-cmake-tools.md)       |
| Test folder         | Refreshed Windows package, repeatable startup checks and test notes                | [Package](agents/handoffs/2026-10-02-first-test-version.md)   |
| E1-E4               | Owned execution data, bounded graph/internal threads and live progress             | [Execution](agents/handoffs/2026-10-02-execution-e4.md)       |
| W1b                 | Strict browser JSON/UTF-8 transport and shared native compatibility tests          | [Transport](agents/handoffs/2026-10-02-web-w1b-transport.md)  |
| Use mode            | Component controls/viewers, shared history/execution and separate saved workspace  | [Use](agents/handoffs/2026-10-03-native-use-mode.md)          |
| Scene S1            | Modular snapshots, stable selection, camera pan/zoom/views and triangle picking    | [S1](agents/handoffs/2026-10-03-scene-s1.md)                  |
| Scene S2            | Move/rotate/scale previews and atomic viewer commands for graph/component controls | [S2](agents/handoffs/2026-10-03-scene-s2.md)                  |
| Inline controls     | Editable numeric fields in graph cards, shared commands and compact socket layout  | [Inline](agents/handoffs/2026-10-03-inline-node-controls.md)  |

## Inline node controls checkpoint

Inline numeric controls are embedded in Build-mode nodes, including collapsed
components' exposed controls. Enter/focus change and spin steps apply through the
shared project history. See [the guide](INLINE_NODE_CONTROLS.md) and
[checkpoint](agents/handoffs/2026-10-03-inline-node-controls.md).
The field layout now uses aligned full-width rows with consistent insets;
see [layout review](agents/handoffs/2026-10-03-inline-node-layout.md).
Numeric rows also include sliders with one undoable edit per drag; see
[slider checkpoint](agents/handoffs/2026-10-03-inline-node-sliders.md).
Slider edits now apply during dragging and trigger live execution, while retaining
one undo step; see [live slider follow-up](agents/handoffs/2026-10-03-live-node-sliders.md).
Parameter wheel changes use a signed exponential curve with Shift for finer
adjustments; see [wheel checkpoint](agents/handoffs/2026-10-03-exponential-parameter-wheel.md).

## Process Gantt checkpoint

Desktop **Process Gantt** widget and browser profiling panel are implemented,
following read-only review of the legacy Gantt display. Queue/invocation timings
remain transient and domain-independent; see [analysis](GANTT_ANALYSIS.md) and
[checkpoint](agents/handoffs/2026-10-02-process-gantt.md). Per-node timing labels and a native results duration column are included; see
[follow-up checkpoint](agents/handoffs/2026-10-02-node-execution-times.md).
The user's manual testing feedback is positive so far; specific Process Gantt
checklist coverage was not reported.

## Next work

E1-E4 are recorded in the execution handoffs below; earlier package/checkpoint
records retain their historical verification counts and limitations.

Execution implementation includes E1 input/output isolation, E2 bounded
sequential/parallel scheduling, E3 native controls/live node/component progress and
E4 managed internal work sharing the run budget. See [execution](EXECUTION.md),
[E4](agents/handoffs/2026-10-02-execution-e4.md),
[E3](agents/handoffs/2026-10-02-execution-e3.md),
[E2](agents/handoffs/2026-10-02-execution-e2.md),
[E1](agents/handoffs/2026-10-02-execution-e1.md) and
[decision 0017](architecture/decisions/0017-native-execution-concurrency.md).

1. Try the included scene/data tools in native Use mode and refine the interface
   from concrete feedback. The user authorized this prototype on 2026-10-03;
   the earlier positive manual feedback applies to the preceding desktop version.
   Real data input/output remains a candidate for a separate next milestone.
2. Address any demonstrated usability or correctness issues in separate verified
   checkpoints. The user's initial manual feedback is positive so far.
3. Record specific desktop checklist coverage when available before marking full
   N5 accepted; this does not block independent product exploration. Bounded graph
   and managed internal parallel work are implemented. See [execution](EXECUTION.md)
   and the [original review](architecture/execution-review-2026-10-02.md).

Nested components, input fan-out interfaces, asset import/bundling/relocation,
production 3D rendering, broader scene hierarchies, richer table schemas and
CSV import remain unimplemented. Public installer/signing, distribution and
clean-machine Windows compatibility remain future work.

## Parallel web development

The approved [web plan](WEB_IMPLEMENTATION_PLAN.md) is complete through W6.
`apps/web` provides retained editing/history, strict import/download, worker
execution and statuses, table/primitive scene viewers, collapsed component
execution/authoring, and browser layout restoration. Core remains domain-independent;
SDK registration and runtime use public contracts; bundled extensions own domain
behavior. This remains a local preview with provisional contracts, not a production
plugin loader.

Production Chromium acceptance covers both domains, components, cancellation,
file reopening and native save → browser edit/download → fresh native reopen.
Unknown extension content, inactive graphs, assets and separate workspace namespaces
survive that exchange. Browser numeric transport restrictions remain explicit under
[decision 0019](architecture/decisions/0019-browser-json-transport.md).
See [delivery and supported-browser evidence](WEB_DELIVERY.md) and
[W6 handoff](agents/handoffs/2026-10-02-web-w6-delivery.md).
Native manual testing has positive user feedback; full checklist acceptance
remains unrecorded independently of browser acceptance.

## Open decisions

Audience/market, project distribution license, broader extension packaging and
migration, asset storage, performance targets, additional operating systems and
remote services remain open. C++/Qt, the copied QtNodes canvas, the initial native
schema-v1 file contract and current source-level SDK are implemented choices;
their long-term compatibility/performance policies remain to be established.

See [architecture](architecture/README.md), [product proposal](product/SmartFlowProposal.md)
and the [historical handoff index](agents/handoffs/README.md). The old checkout is
never a build/runtime dependency and must remain unchanged.
