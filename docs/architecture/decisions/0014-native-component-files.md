# 0014: Standalone native component files

Date: 2026-10-02
Status: accepted for the native migration preview (N6e)

## Context

Definitions currently live only in a project's catalog. Reuse between projects
needs an explicit file exchange operation that preserves identity, opaque content
and reproducible snapshots without introducing a package loader.

## Decision

A `.smartflow-component` file contains one existing schema-v1
`smartflow.graph-component` definition directly, without a new envelope. `.json`
files are accepted too. Its graph, exposed interface and saved control defaults
are retained exactly. It carries no workspace, destination bindings or project
asset store. Asset bundling and URI relocation are outside this checkpoint.

Reuse the project's strict JSON transport (16 MiB and nesting limits, duplicate-key
and integer-overflow rejection, finite values, atomic writes without direct-write
fallback). Project entry points still validate the complete project schema;
component entry points validate their own definition contract. Neither transport
imports domains or requires installed node packages.

Import uses the existing transactional catalog command: matching content is a
no-op, conflicting content under the same identity/version rejects, and a new
entry is one undoable edit. It never inserts nodes or modifies existing instance
snapshots. Export validates structure before opening the destination and leaves
the project, history and active project path unchanged. Unavailable packages may
be exchanged; future or malformed definition contracts remain retained in project
files but cannot be imported/exported as supported standalone definitions.

The library exposes Import and Export buttons, including Import in an empty
library. Errors remain inline; cancellation performs no file or project mutation.
Revision checks reject stale dialogs before file writes or catalog changes.

## Consequences

Users can reuse a component across projects without a cloud service or domain
dependency. Installing node packages is a separate concern, and importing does
not establish executable support. Nested definitions, editing/deletion, external
assets, migrations and automatic updates remain future work. This adds no
production plugin system, installer or new dependency.
