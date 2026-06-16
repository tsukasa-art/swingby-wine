# Melammu Wine Patch Policy (swingby-wine)

Last updated: 2026-06-15

This repository is the canonical Wine fork for Melammu. The git base is the
**vanilla WineHQ Wine 10.0 release** (`b0738596` "Release 10.0.", Alexandre
Julliard); all macOS/Rosetta work is carried by this fork's own patches
(36 commits / 22 files). Sikarugir (formerly Kegworks) is historical lineage
and credit only — measured source delta over WineHQ 10.0 in this repo is zero
(no Sikarugir commits are carried here). WineHQ is the upstream to rebase onto.
Melammu runtime behavior must not depend on untracked binaries copied from
third-party wrappers.

## Responsibility Boundary

`swingby-wine` owns:

- Wine behavior patches.
- Patch gates and default-off behavior.
- Regression surface and compatibility notes for each patch class.
- Reproducible build inputs for artifacts shipped by Melammu.

`Melammu` owns:

- Prefix lifecycle and game data lifecycle.
- Engine detection and per-engine policy.
- Runtime selection, `WINEPREFIX`, `WINEDLLPATH`, GStreamer, DXVK, and DLL
  override policy.
- Per-title feature flags.
- The bundled runtime manifest under `docs/wine-runtime-manifest.md`.

## Canonical Branch Policy

Target policy:

- Public default / canonical branch: `master` (Wine fork convention; no `main` branch).
- `master` contains Melammu's supported Wine patch set.
- Phase 0 authority is the runtime content currently used by the Melammu
  launcher, not an existing branch name or version string.
- v1 `master` is the minimal clean fork needed to reproduce the current Melammu
  bundle (no Wine version upgrade; stays on the WineHQ 10.0 base).
- Experimental game-specific probes live on topic branches and are not bundled
  until promoted by a manifest update.
- Launcher state, agent state, local research notes, and build directories do
  not belong in public history.

Current Phase 0 source candidate:

```text
e6310c904ca5ddc9d8a78017057ba9f661c2f57a
feat(d3d9,wined3d): MELAMMU_CMVS_THUMBS ...
```

This commit is a candidate source explanation for the current Melammu bundle,
but the bundle is authoritative until a clean rebuild proves equivalence by
feature probes and hashes.

v1 `master` includes:

- Core patches required by the current Melammu bundle.
- `MELAMMU_CMVS_THUMBS` as a default-off engine gate.

v1 `master` excludes:

- SoftDenchi WTS / crypt32 work.
- Wine version upgrades.
- `wine64/` integration into this fork (wine64 stays a separate
  launcher-bundled compatibility runtime under `wine-support/wine64/`).
- Title policy, fallback policy, and launcher policy.

SoftDenchi-related work after that point is experimental/post-v1 and must not
be included in the canonical v1 runtime without a separate gate:

```text
57cd6848acc... wtsapi32 WTS session nudge
643e95ec332... crypt32 CryptBinaryToStringA(CRYPT_STRING_HEX)
```

## Patch Classes

### Core Required

These patches are part of the supported Melammu Wine runtime and should be
carried by the canonical branch.

- `dlls/wow64cpu/cpu.c`: Rosetta-aware far-call thunking for 32-bit games.
- `dlls/ntdll/loader.c`: make delay-load IAT writable before patching.
- `dlls/ntdll/unix/virtual.c`: avoid initial executable file mappings on
  macOS, then apply executable protection explicitly.
- `dlls/win32u/window.c`: guard self-referencing window subclass updates.
- `dlls/winemac.drv/*`: macOS window behavior and capture support used by
  Melammu.

### Engine Gated

These patches must be default-off inside Wine and enabled by Melammu only for
the engine/title policy that requires them.

- CMVS thumbnail capture and last-presented back-buffer serving.
- Gate: `MELAMMU_CMVS_THUMBS`.
- IPC contract: `/tmp/melammu_snap_NNN.bgra` with
  `[u32 width][u32 height][u32 stride] + BGRA pixels`, plus
  `/tmp/melammu_page_base.txt`.

### Experimental / Post-v1

These patches are not part of the v1 canonical runtime until explicitly
promoted.

- `dlls/wtsapi32/wtsapi32.c`: WTS session nudge for SoftDenchi experiments.
- `dlls/crypt32/base64.c`: `CryptBinaryToStringA(CRYPT_STRING_HEX)` work until
  promoted as a generic Wine compatibility fix.

## Build Artifact Rule

The shipped Melammu runtime must be tied back to source by content, not by
mtime. Before publishing or rebasing:

1. Build from the intended canonical commit.
2. Bundle into Melammu.
3. Record the commit, feature probes, and SHA-256 hashes in Melammu's
   `docs/wine-runtime-manifest.md`.
4. Verify that experimental patches are absent unless the manifest explicitly
   says otherwise.
