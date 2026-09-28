# Checkpoint N1: copied native canvas and C++/Qt shell

## Objective and scope

Implement the user's corrected direction: native C++/Qt based on copies from
the old repository. TypeScript remains available for a future extension.
Never modify or delete the old repository.

## Current state

- Copied QtNodes headers, sources and BSD-3-Clause license to `native/vendor`.
- SHA-256 provenance recorded in `native/vendor/copy-manifest.json`.
  Only `Globals.h` differs: Qt export macros replace the platform dependency.
- Independent CMake build produces `smartflow.exe` and `native_tests.exe`.
- Native canvas supports a numeric migration harness with connected nodes.
- Updated agent guidance, roadmap, proposal status and architecture direction.
- All previous TypeScript work and the historical reuse analysis are preserved.

## Verification

- Configured with CMake 3.23, VS2022 x64 and Qt 6.11.1 MSVC kit.
- Release build passed. Compiler child processes required sandbox permission.
- CTest passed connection propagation/disconnection and canvas snapshot tests.
- Offscreen application startup passed.
- Old checkout status remains its pre-existing untracked
  `documentation/VisualWorkspaceProposal.md`; no source changes made there.

Final cycle-rejection, render and web-regression results are recorded after the
final check. See [native development](../../NATIVE_DEVELOPMENT.md) for commands.

## Decisions and limitations

See [native-first decision](../../architecture/decisions/0003-native-first.md).
This is a native canvas migration checkpoint, not the complete studio migration.
The numeric adapter is new harness code; the QtNodes library is copied code.
QtNodes snapshots are tested internally but are not a stable SmartFlow save
format. Real desktop input/GPU interaction and standalone deployment are not
verified by offscreen testing. Pipeline, inspector and 3D engine remain pending.

## Next steps

N2: audit and copy pipeline/data/task dependencies, retain notices and add
native behavior tests. N3: migrate workspace/inspector. N4: migrate the 3D
workflow. N5: native persistence and independent non-3D workflow. Record and
verify each checkpoint before proceeding.
