# 0023: Native Use mode from component interfaces

Status: accepted, 2026-10-03.

## Context

The user wants to continue developing a generic platform without choosing the
final customer or domain. Collapsed components already expose controls and
outputs. A person operating a prepared workflow should be able to adjust those
controls and inspect results without navigating the graph editor.

## Decision

Add Build/Use modes to the existing native workspace. Use mode selects a collapsed
component instance from the active graph and presents only its public controls
and outputs, using the configured extension's viewer factory. The source-level
factory can supply independent viewer instances for Build and Use.

Both modes share the retained project, semantic command history and background
executor. Apply uses the existing parameter command; Run executes the complete
graph, including the sources of component inputs. Revision checks, live updates,
cancellation and diagnostics retain their existing semantics.

Mode, tool/output selection and opaque viewer state are per-graph workspace data
in `smartflow.native-use@1`. Build layout/navigation, selection and pinned output
remain separate. The hidden Build viewport must not overwrite saved navigation.
No project or component contract version change is required.

## Alternatives

A new graph-level form schema could expose arbitrary ordinary-node parameters,
but would duplicate existing component interface authoring for this experiment.
Reusing the inspector alone would still require graph-node selection and would
expose implementation parameters. Exporting a standalone application would add
packaging and compatibility work before testing the control/viewer interaction.

## Consequences and verification

The first implementation supports existing numeric editors and one selected
component/output viewer. Unsupported content remains retained with diagnostics;
missing tools show preparation guidance. Use is a presentation mode, not a
permission boundary. No dynamic plugin loader, new backend, browser Use mode or
application exporter is introduced.

Native tests cover command history, live and explicit runs, obsolete editors,
cancellation, diagnostic visibility, empty graphs, unknown workspace content,
mode/layout retention, scene and data viewers, and save/reopen. Installed startup
screenshots complement offscreen interaction coverage. Manual acceptance of this
new interface remains distinct from the user's positive feedback on the previous
desktop version.
