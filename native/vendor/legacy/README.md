# N2 copied pipeline foundation

Selected sources copied on 2026-09-28 from the old studio engine checkout at
commit `d6f457e8415445d00eaf7e9fbf9e5047768ea637`. The provenance source was
`D:/dev/omi_code/studio_engine`; that directory is never needed by the build or
runtime. `copy-manifest.json` records the source and current copied SHA-256 for
each imported file. `CMakeLists.txt` is new SmartFlow build glue, not an imported
legacy build file.

## Audit and selection

Reviewed module dependency lists, source include closure, pipeline mapping and
dependency resolution, parameter fixup, collection ownership, task queue
execution/shutdown, and module/file license notices. This is a migration audit,
not an exhaustive security review or a certification of all copied APIs.

| Module | Copied purpose | Retained notice |
| --- | --- | --- |
| tp_pipeline | Step/parameter/port model, complex-object dependency, delegate registry, manager, step output/profiling, None delegate | MIT, LICENSE |
| tp_data | Collections, factories, numeric/string/file members | MIT, LICENSE |
| tp_task_queue | Task queue, synchronization and work-queue helpers | MIT, LICENSE |
| tp_utils | Required utility source subset and transitive headers | MIT, LICENSE |
| lib_platform | Random device/thread naming source and required headers | MIT, LICENSE |
| lib_json | Required nlohmann headers | MIT, LICENSE.MIT |
| lib_date | date.h only; no timezone service or source | Per-file MIT notice and LICENSE.txt |
| lib_stduuid | UUID header and four required GSL headers | MIT, LICENSE; Microsoft notices retained in GSL headers |
| lib_base64 | Parameter binary-value helper | zlib-style notice, LICENSE |

The explicit CMake source list builds static libraries without the original
qmake/CMake tooling, generated configuration, binaries, credentials, or assets.
Windows compiler definitions select the copied platform branches and static
exports. No domain library is linked by the execution adapter.

`tp_data` lists `tp_utils_filesystem` in its old module manifest, but this selected
source closure uses `tp_utils/FileUtils` directly. The separate filesystem module
is not needed. Pipeline image dependencies originate in the results manager,
which is not selected. Wrapper/library/optimizer modules, image utilities,
pipeline widgets, and scene modules are omitted. Optional date timezone sources
and platform utilities outside the required include closure are omitted too.

## Adaptation

Only `tp_pipeline/src/PipelineManager.cpp` differs from the imported sources.
It retains the constructor's parameter-fixup policy across restarts/callbacks and
does not clear dangling inputs when fixup is disabled. The isolated patch is in
`patches/0001-preserve-no-fixup.patch`. All imported notices and vendor formatting
are preserved. Carry these licenses and embedded notices into distributions.

Use the SmartFlow adapter for execution. The raw legacy API can mutate parameters,
prune connections, share mutable data members, and read/write legacy binary
formats; those APIs are not the SmartFlow project contract. See
[the execution decision](../../../docs/architecture/decisions/0004-native-pipeline-migration.md).
