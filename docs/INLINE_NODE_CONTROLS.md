# Inline native node controls

Build mode shows a node's numeric parameters directly inside its graph card.
Sockets appear above full-width rows with labels left and values right. Rows use
the same inset and spacing, with quiet execution timing below. Ordinary
nodes follow their delegate's parameter order; collapsed components follow their
authored public-control order. Unknown node content stays unavailable and retained.

Each numeric row includes a horizontal slider. Dragging applies live changes to
the graph and recomputes with Live enabled, with updates bounded to about 25 per
second. Release flushes the final value; the entire drag remains one undo step.
Escape restores the starting value. Slider keyboard steps and track clicks apply immediately.
Typing, undo and edits in other widgets keep the slider synchronized. An external
project change cancels an unfinished drag rather than overwriting the newer value.
The slider uses the declared bounds; domains wider than 10,000 use a smaller
window around the current value. Typing outside that window moves it, without
restricting the numeric field's full allowed range. Its tooltip shows the window.

Click a value, type and press Enter, or leave the field, to apply it. Up/Down
applies each step immediately. Escape discards unfinished typing. No
Apply button is required. An unfocused field ignores the wheel so canvas zoom does
not accidentally edit values; a focused field can use the wheel to adjust them.

Edits share project commands, undo/redo, dirty tracking and background execution
with the inspector, Use mode and viewers. Live updates recompute after committed
edits; with Live disabled, use Run. The inspector remains available for selected
node details and its existing controls. Use mode retains its public tool interface.

Typing stays local until committed. Semantic edits from another widget refresh the
controls and discard unfinished drafts. Execution timing updates do not replace
the parameter widgets or their draft text. Controls are bound to stable node IDs,
never long-lived StepDetails pointers. Declared ranges and enabled state apply.
Unsupported parameter types remain read-only; the current bundled nodes expose
numeric parameters.

`NodeParameterPanel` and `NodeBodyGeometry` live under `native/app/workspace`.
They have no scene imports. A small documented QtNodes customization hook allows
application-owned geometry; vendor default geometry and file semantics remain
available. Socket hit testing and connection endpoints share that geometry.
No project schema or workspace position/navigation migration is needed.

Tests exercise actual proxy-widget canvas clicks/typing, execution, draft cancel,
external updates, no movement while editing, undo/redo, component isolation,
deletion/unknown content and save/reopen. Offscreen renders and installed Windows
startup checks support verification; manual desktop acceptance remains separate.
