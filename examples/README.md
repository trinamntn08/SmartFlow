# Native example projects

These schema-v1 files contain deterministic graphs and workspace configuration,
without external assets or private data. They are copied into the desktop test
folder by CMake install. Open them through File > Open or launch with `--project`.

From the repository root after preparing the desktop folder:

```powershell
build/desktop-test/bin/smartflow.exe --project "$PWD/build/desktop-test/examples/scene.smartflow"
build/desktop-test/bin/smartflow-data.exe --project "$PWD/build/desktop-test/examples/data.smartflow"
build/desktop-test/bin/smartflow-data.exe --project "$PWD/build/desktop-test/examples/components.smartflow"
```

`scene.smartflow` restores Cube size 3, an orbited camera (yaw 60, pitch 15),
the selected object and pinned final Scene output. `data.smartflow` restores
minimum 30, count 2, total 79, mean 39.5 and the selected Total row.

`components.smartflow` shows Sample table connected to one Filtered summary
component node. Its minimum control is 30, with the exposed summary output pinned
(count 2, total 79, mean 39.5). Change minimum in the inspector, use Undo/Redo, or
choose rows in the Output selector and click Pin output. The saved library can
insert additional instances without changing the original definition.

The primary executable can also use `--data --project <data-file>`. Select the
matching application configuration; the current preview does not infer or load
extensions from the project file. Unavailable content remains retained and
execution is blocked with diagnostics.

Save As to a separate local file while testing. The project path flag reports
missing/unreadable files and exits with code 6. Append `--smoke-test` for a
startup/execution check, which exits 5 when the loaded graph cannot execute.
These checks do not replace real desktop interaction or clean-machine testing.

`filtered-summary.smartflow-component` contains the standalone definition from
`components.smartflow`. In the data app, open **Components > Component library**,
click **Import component...**, choose this file, bind `table` to Sample table / out
and insert it. Export saves the definition defaults, separately from instance edits.
