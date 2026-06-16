# swingby-wine

A personal Wine fork targeting macOS (Apple Silicon / Rosetta 2), built for use with the [Melammu](https://github.com/tsukasa-art/Wukiyo) launcher. Based on the [WineHQ](https://gitlab.winehq.org/wine/wine) **Wine 10.0 release** (`b0738596`), it carries this fork's own macOS/Rosetta compatibility patches. This repository is the source of truth for Melammu's bundled Wine runtime; WineHQ is the upstream to rebase onto.

Inspired by the macOS Wine work of [Sikarugir](https://github.com/Sikarugir-App/Creator) (formerly Kegworks). See [SWINGBY_PATCHES.md](SWINGBY_PATCHES.md) for the responsibility boundary and patch classification.

## Patches

### Core runtime patches

The supported Melammu runtime carries macOS/Rosetta compatibility work in
`wow64cpu`, `ntdll`, `win32u`, and `winemac.drv`. These are not launcher
features; they are Wine behavior fixes and belong in this fork.

### `d3d9` / `wined3d` — CMVS thumbnail capture

CMVS save/load thumbnails are gated by `MELAMMU_CMVS_THUMBS` and must remain
default-off for non-CMVS engines.

The current mechanism serves the last-presented frame to back-buffer
`LockRect(READONLY)` calls and uses Melammu-provided snapshot files only through
the documented launcher/Wine IPC contract.

Snap file format:

```text
[u32 width][u32 height][u32 stride] + BGRA pixels (top-down)
```

**Affected files**: `dlls/d3d9/*`, `dlls/wined3d/swapchain.c`

## Branches

| Branch | Base | Contents |
|---|---|---|
| `master` | WineHQ Wine 10.0 release | **canonical** — supported Melammu Wine patch set (Wine fork convention; no `main`) |
| `softdenchi-wts-session` | `master` | experimental/post-v1 (SoftDenchi WTS / crypt32). Not bundled unless promoted |
| `backup/pre-purge-master-*` | — | historical reference (pre-rehab master snapshot) |

## Build (macOS / Rosetta 2)

Requires Xcode Command Line Tools and x86_64 Homebrew (`/usr/local/bin/brew`).

```bash
mkdir build && cd build
arch -x86_64 env \
  PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/usr/local/share/pkgconfig \
  LDFLAGS="-L/usr/local/lib" CPPFLAGS="-I/usr/local/include" \
  ../configure -C --enable-win64 --with-mingw \
  BISON=/usr/local/opt/bison/bin/bison
arch -x86_64 make -s -j$(sysctl -n hw.activecpu)
```

Produces x86_64 binaries that run under Rosetta 2.

## Deploy into Melammu

The built `.so` files replace their counterparts inside `Melammu.app/Contents/Resources/wine-support/`. ABI must match — build from the `master` branch (WineHQ Wine 10.0 base).

```bash
WINE_LIB=~/Applications/Melammu.app/Contents/Resources/wine-support/wine/lib/wine/x86_64-unix

# winemac patch
cp build/dlls/winemac.drv/winemac.so "$WINE_LIB/winemac.so"
codesign --force --sign - "$WINE_LIB/winemac.so"

# d3d9 thumbnail injection
cp build/dlls/d3d9/d3d9.dll.so "$WINE_LIB/d3d9.dll.so"
codesign --force --sign - "$WINE_LIB/d3d9.dll.so"
```

Alternatively, use the `deploy.sh` / `deploy_d3d9.sh` scripts in the Melammu repo which handle paths and re-signing automatically.

**Note on notarization**: Ad-hoc re-signing (`-`) is sufficient for running on your own Mac (with Terminal listed under Privacy & Security → Developer Tools). Distribution requires a Developer ID certificate and full re-notarization of Melammu.app.

## Notes on D3D9

For D3D9 games (KiriKiriZ, etc.), [d9vk](https://github.com/Joshua-Ashton/d9vk) (D3D9→Vulkan→MoltenVK→Metal) is recommended over Wine's built-in wined3d. Melammu bundles d9vk and writes `drive_c/dxvk.conf` with `d3d9.presentInterval = 1` at install time to force vsync — without this, KiriKiriZ engines write `waitvsync=no` to their `.cfu` config on first run, causing flickering on Wine+Metal.

D3DMetal (Apple GPTK) covers D3D11/D3D12/DXGI/DDraw but **not** D3D9; d9vk remains the correct path for D3D9 titles.

## Related

- [Melammu](https://github.com/tsukasa-art/Wukiyo) — macOS launcher and HUD that drives the thumbnail injection
- [Sikarugir](https://github.com/Sikarugir-App/Creator) — macOS Wine work that inspired this project
- [Zenn: Mac で美少女ゲームを動かす](https://zenn.dev/tsukasa_art/articles/mac-eroge-compat-part1) — series documenting the compatibility work

---

## INTRODUCTION

Wine is a program which allows running Microsoft Windows programs
(including DOS, Windows 3.x, Win32, and Win64 executables) on Unix.
It consists of a program loader which loads and executes a Microsoft
Windows binary, and a library (called Winelib) that implements Windows
API calls using their Unix, X11 or Mac equivalents.  The library may also
be used for porting Windows code into native Unix executables.

Wine is free software, released under the GNU LGPL; see the file
LICENSE for the details.


## QUICK START

From the top-level directory of the Wine source (which contains this file),
run:

```
./configure
make
```

Then either install Wine:

```
make install
```

Or run Wine directly from the build directory:

```
./wine notepad
```

Run programs as `wine program`. For more information and problem
resolution, read the rest of this file, the Wine man page, and
especially the wealth of information found at https://www.winehq.org.


## REQUIREMENTS

To compile and run Wine, you must have one of the following:

- Linux version 2.6.22 or later
- FreeBSD 12.4 or later
- Solaris x86 9 or later
- NetBSD-current
- Mac OS X 10.12 or later

As Wine requires kernel-level thread support to run, only the operating
systems mentioned above are supported.  Other operating systems which
support kernel threads may be supported in the future.

**FreeBSD info**:
  See https://wiki.freebsd.org/Wine for more information.

**Solaris info**:
  You will most likely need to build Wine with the GNU toolchain
  (gcc, gas, etc.). Warning : installing gas does *not* ensure that it
  will be used by gcc. Recompiling gcc after installing gas or
  symlinking cc, as and ld to the gnu tools is said to be necessary.

**NetBSD info**:
  Make sure you have the USER_LDT, SYSVSHM, SYSVSEM, and SYSVMSG options
  turned on in your kernel.

**Mac OS X info**:
  You need Xcode/Xcode Command Line Tools or Apple cctools.  The
  minimum requirements for compiling Wine are clang 3.8 with the
  MacOSX10.10.sdk and mingw-w64 v8.  The MacOSX10.14.sdk and later can
  only build wine64.

**Supported file systems**:
  Wine should run on most file systems. A few compatibility problems
  have also been reported using files accessed through Samba. Also,
  NTFS does not provide all the file system features needed by some
  applications.  Using a native Unix file system is recommended.

**Basic requirements**:
  You need to have the X11 development include files installed
  (called xorg-dev in Debian and libX11-devel in Red Hat).
  Of course you also need make (most likely GNU make).
  You also need flex version 2.5.33 or later and bison.

**Optional support libraries**:
  Configure will display notices when optional libraries are not found
  on your system. See https://gitlab.winehq.org/wine/wine/-/wikis/Building-Wine
  for hints about the packages you should install. On 64-bit
  platforms, you have to make sure to install the 32-bit versions of
  these libraries.


## COMPILATION

To build Wine, do:

```
./configure
make
```

This will build the program "wine" and numerous support libraries/binaries.
The program "wine" will load and run Windows executables.
The library "libwine" ("Winelib") can be used to compile and link
Windows source code under Unix.

To see compile configuration options, do `./configure --help`.

For more information, see https://gitlab.winehq.org/wine/wine/-/wikis/Building-Wine


## SETUP

Once Wine has been built correctly, you can do `make install`; this
will install the wine executable and libraries, the Wine man page, and
other needed files.

Don't forget to uninstall any conflicting previous Wine installation
first.  Try either `dpkg -r wine` or `rpm -e wine` or `make uninstall`
before installing.

Once installed, you can run the `winecfg` configuration tool. See the
Support area at https://www.winehq.org/ for configuration hints.


## RUNNING PROGRAMS

When invoking Wine, you may specify the entire path to the executable,
or a filename only.

For example, to run Notepad:

```
wine notepad            (using the search Path as specified in
wine notepad.exe         the registry to locate the file)

wine c:\\windows\\notepad.exe      (using DOS filename syntax)

wine ~/.wine/drive_c/windows/notepad.exe  (using Unix filename syntax)

wine notepad.exe readme.txt          (calling program with parameters)
```

Wine is not perfect, so some programs may crash. If that happens you
will get a crash log that you should attach to your report when filing
a bug.


## GETTING MORE INFORMATION

- **WWW**: A great deal of information about Wine is available from WineHQ at
	https://www.winehq.org/ : various Wine Guides, application database,
	bug tracking. This is probably the best starting point.

- **FAQ**: The Wine FAQ is located at https://gitlab.winehq.org/wine/wine/-/wikis/FAQ

- **Wiki**: The Wine Wiki is located at https://gitlab.winehq.org/wine/wine/-/wikis/

- **Gitlab**: Wine development is hosted at https://gitlab.winehq.org

- **Mailing lists**:
	There are several mailing lists for Wine users and developers; see
	https://gitlab.winehq.org/wine/wine/-/wikis/Forums for more
	information.

- **Bugs**: Report bugs to Wine Bugzilla at https://bugs.winehq.org
	Please search the bugzilla database to check whether your
	problem is already known or fixed before posting a bug report.

- **IRC**: Online help is available at channel `#WineHQ` on irc.libera.chat.
