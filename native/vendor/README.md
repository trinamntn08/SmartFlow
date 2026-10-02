# Copied native dependencies

`qtnodes/inc`, `qtnodes/src`, and `qtnodes/LICENSE.rst` were copied from
`D:/dev/omi_code/studio_engine/lib_qtnodes` at repository HEAD
`d6f457e8415445d00eaf7e9fbf9e5047768ea637` on 2026-09-27.
The source checkout is never needed to build SmartFlow.

Git preserves files under `native/vendor` byte-for-byte so checkout line-ending
conversion does not invalidate the recorded provenance hashes.

The BSD-3-Clause notice is retained in `qtnodes/LICENSE.rst` and must accompany
binary distributions. This is the legacy repository's QtNodes variant, not a
claim that it matches a particular upstream release.

Local adaptation: `inc/QtNodes/Globals.h` replaces the `lib_platform` export
macros with Qt export macros and adds `SMARTFLOW_QTNODES_STATIC`. The new CMake
target compiles the copied sources without the old `tp_build` scripts. N5e also
adds isolated scene command-routing hooks; see
[the local patch notes](qtnodes/SMARTFLOW_PATCHES.md). The copy manifest records
the original source hashes, not the locally adapted hashes.

No private assets, application configuration, native binaries or credentials
were copied. N3 adds selected pipeline widgets under `pipeline-widgets/`; N4 adds
audited lower-level geometry/material math under `scene-math/`. The full legacy
scene application and renderer are not copied. Each module has separate provenance,
licenses and audit notes; the new extension glue lives outside vendor sources.

N2 adds a selected pipeline/data/task foundation under `legacy/`, with its own
[dependency/license audit](legacy/README.md), hash manifest, and isolated local
patch. The original QtNodes provenance above remains separate.
