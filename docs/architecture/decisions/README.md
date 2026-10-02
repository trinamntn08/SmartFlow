# Architecture decision index

Updated: 2026-10-02. Records preserve rationale and scope at their dates. Earlier
pending statements are historical where later decisions/checkpoints implement the
feature. [Current architecture](../README.md) and [roadmap](../../ROADMAP.md) describe
what runs today; this index is not a claim of completed production delivery.

## Decisions

- [0001: Independent browser development scaffold](0001-bootstrap.md)
- [0002: Initial project document and extension contracts](0002-project-contract.md)
- [0003: Native C++/Qt first, TypeScript deferred](0003-native-first.md)
- [0004: Isolate the copied pipeline behind an execution adapter](0004-native-pipeline-migration.md)
- [0005: Native workspace and execution integration](0005-native-workspace.md)
- [0006: Bundled native scene extension](0006-native-scene-extension.md)
- [0007: Native project-file foundation](0007-native-project-files.md)
- [0008: Retained-document editor commands](0008-retained-editor-commands.md)
- [0009: First user test release is the Windows desktop app](0009-desktop-test-release.md)
- [0010: Native workspace persistence](0010-native-workspace-persistence.md)
- [0011: Independent native data workflow](0011-independent-native-data-extension.md)
- [0012: Native graph component foundation](0012-native-graph-component-foundation.md)
- [0013: Collapsed native component snapshots](0013-collapsed-native-components.md)
- [0014: Standalone native component files](0014-native-component-files.md)
- [0015: Component editing and explicit update scope](0015-component-editing-scope.md)

0003 selects native C++/Qt, superseding earlier browser implementation proposals.
0009 requires packaged Windows desktop acceptance, which remains pending manual
results. N5f/N5g complete file actions/workspace persistence after their earlier
foundation decisions. 0013 implements collapsed components after 0012's expanded
insertion; 0014 and its N6f follow-up implement standalone exchange and removal.
0015 governs implemented editing copies followed by planned explicit compatible
snapshot replacement. General migration and revision policy remain open.

Use [the template](TEMPLATE.md) for consequential new decisions. Do not rewrite
a dated decision's original rationale to imply it already contained later work.
