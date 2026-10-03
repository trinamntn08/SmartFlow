# Task handoff: exponential parameter scrolling

## Objective and scope

The user requested smoother exponential scrolling and clarified that this applies
to parameter values. Native inline numeric fields and sliders share the change.

## Current state

Wheel adjustment transforms the value with `asinh(value / scale)`, adds a wheel
delta, then transforms back with `scale * sinh(...)`. The scale is the declared
parameter step divided by 0.05. This yields approximately the declared step near
zero and proportional changes at large magnitudes. The signed curve remains
continuous through zero and negative values; inverse wheel movements reverse it
within numeric precision unless a bound was reached.

Wheel notches use angle delta / 120; trackpad input uses pixel delta / 40. Shift
scales adjustments by 0.1. Numeric bounds, project commands, undo and live
execution apply normally. The slider routes wheel changes through its precise
numeric editor to avoid quantizing them to integer slider positions. Unfocused
controls leave wheel input to the canvas. Keyboard steps and zoom are unchanged.

## Verification

- `cmake --build --preset windows-local-release`: passed.
- With `SMARTFLOW_TEST_FONT=C:/Windows/Fonts/segoeui.ttf`,
  `ctest --preset windows-local-release-tests -j 4`: all 32 entries passed.
  New wheel tests activate the window and focus via the embedded canvas; they
  cover zero/negative values, inverse deltas, magnitude scaling, Shift, fractional
  wheel/trackpad deltas, exact field/slider synchronization, bounds and undo.
  Unfocused fields leave the project unchanged.
- `npm.cmd run check`: formatting, types, workspace tests and build passed.
- `cmake --install build/native --config Release --prefix build/desktop-test`
  and `scripts/Test-DesktopPackage.ps1`: all 13 installed Windows startup/render
  checks passed; installed executable hashes match the Release build.
- Inspected the packaged numeric render; inline controls remain correctly placed.
- `git diff --check`: passed. No old-repository edits.

## Decisions and open questions

No package/schema changes. Manual feedback on wheel sensitivity remains unreported.

## Next steps

Try focused field/slider scrolling, Shift and trackpad input with representative
small, large, zero and negative values.
