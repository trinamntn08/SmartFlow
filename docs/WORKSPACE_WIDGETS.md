# Arrange workspace widgets

Every region has a widget selector and three controls:

- **H** splits into left and right regions.
- **V** splits into top and bottom regions.
- **X** closes that region. The widget remains available in the selectors.

Choose **Graph**, **Node library**, **Inspector**, **Execution results** or
**Result viewer** in any region. Choosing a widget that is already visible swaps
the two regions. Drag a divider to resize. You can create up to 32 regions;
there is one live instance of each widget. The numeric configuration has no
Result viewer. Node creation is now in the **Node library** widget.

Use **Layout > Reset widget layout** to recover the default arrangement. Closing
all but one region leaves a selector so you can rebuild the layout.

**Save/Save As** stores the region arrangement and divider proportions with the
project. **Open** restores them, along with graph navigation, selection and viewer
settings. Layout changes mark workspace state dirty without adding graph undo
commands or executing the graph. Moving/hiding a widget preserves its live state;
the graph keeps its zoom rather than refitting when moved.

Old projects use the default arrangement until saved. Unavailable widgets keep
their saved placeholders. Unsupported future/malformed whole layouts remain
retained while a default layout is shown; explicit Reset replaces their layout
state. Outer window size/position and floating/dock windows are not part of this
checkpoint. See [decision 0016](architecture/decisions/0016-selectable-workspace-widgets.md).
