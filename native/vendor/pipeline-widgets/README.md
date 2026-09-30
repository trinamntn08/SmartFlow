# N3 copied pipeline widgets

Eight files from `tp_qt_pipeline_widgets` in the old studio engine checkout,
commit `d6f457e8415445d00eaf7e9fbf9e5047768ea637`. Historical source location:
`D:/dev/omi_code/studio_engine/tp_qt_pipeline_widgets`. Neither the build nor
runtime accesses that location. `copy-manifest.json` records source and copy
SHA-256 hashes; all eight copies are unchanged. The module's proprietary,
confidential, all-rights-reserved notice is preserved in `LICENSE`; these files
are not MIT-licensed. This migration does not grant redistribution rights or
change the repository's undecided distribution license.

Reviewed the parameter editor base, double editor, delegate-to-port adapter,
ownership/destructors, includes, and their dependency closure. Only these small
widgets and their headers are included; the old workspace, global style setup,
pipeline manager UI, persistence, and application configuration are excluded.
This is a migration audit, not a comprehensive security review.

SmartFlow composes a new workspace around the copied controls. Parameter editors
return a draft value; explicit project commands apply edits through one undo
stack. The canvas subclass overrides anchor writes to keep routing geometry out
of semantic project state. Runtime values are provided by the N2 executor, not
the copied canvas placeholder data. These adaptations live in `native/app`, so
vendor formatting and behavior remain unchanged.
