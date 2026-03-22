# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Double Action: Boogaloo is a Source Engine multiplayer game mod (Steam App ID 317360). It compiles two DLLs — `client.dll` and `server.dll` — which are loaded by the Source Engine at runtime from `mp/game/dab/bin/`.

## Build System

The build chain is: **VPC scripts → Visual Studio 2013 .vcxproj files → MSBuild 12.0**.

VPC (Valve's proprietary project file generator) reads `.vpc` files and emits `.vcxproj`/`.sln` files. You rarely need to regenerate unless `.vpc` files change. The pre-generated `DoubleAction.sln` covers all game code.

**Regenerate project files** (only needed when `.vpc` files change):
```bat
cd mp/src
creategameprojects.bat
```
This also embeds the current git commit hash as `DA_GIT_VERSION` in the preprocessor defines.

**Compile debug build:**
```bat
cd mp/src
compiledebug.bat
```

**Compile release build:**
```bat
cd mp/src
compilerelease.bat
```

Both scripts invoke MSBuild 12.0 at `C:\Program Files (x86)\MSBuild\12.0\Bin\MSBuild.exe` against `DoubleAction.sln`. Output DLLs land in `mp/game/dab/bin/`.

**Run the game locally against your built DLLs:**
```bat
cd mp/src
runlocalbuild.bat
```
This launches `hl2.exe` (from your Steam install) pointed at `mp/game/dab` with `-dev +sv_cheats 1 +map da_megachat`.

## Code Architecture

### DLL boundary: shared vs client vs server

All DA game code lives under `mp/src/game/`:

| Directory | Compiled into | Purpose |
|-----------|--------------|---------|
| `game/shared/sdk/` | both DLLs | Player state, game rules, movement, weapons, mini-objectives |
| `game/server/sdk/` | `server.dll` | Authoritative game logic, bots, entity spawning |
| `game/client/sdk/` | `client.dll` | Prediction, HUD, spectator UI |

The pattern `#ifdef CLIENT_DLL` / `#else` is used throughout shared code to compile the same `.cpp` file into both DLLs with different behavior. Class aliases like `#define CSDKGameRules C_SDKGameRules` are the standard way shared headers resolve the client/server split.

### Key shared types (`mp/src/game/shared/sdk/da.h`)

`da.h` is the central enum/define header included everywhere. It defines:
- `announcement_t` — style kill announcements (dive kill, slide kill, etc.)
- `notice_t` — HUD notices (bounty, rat race, briefcase events)
- `miniobjective_t` — active mini-objective modes (briefcase, bounty, rat race)
- `WT_*` — weapon type categories
- Style point/sound enums

### Game rules (`sdk_gamerules.cpp/.h`)

`CSDKGameRules` (server) / `C_SDKGameRules` (client) drives all round logic: mini-objective selection, wanted/bounty system, rat race waypoint tracking, slowmo time management, and round state. This is the first place to look for any mode-level logic.

### Player (`sdk_player.cpp/.h`, `sdk_player_shared.cpp/.h`, `player.cpp`)

The player class hierarchy:
- `CSDKPlayer` (server, `sdk_player.cpp`) — authoritative health, wanted meter, style meter, death/spawn
- `C_SDKPlayer` (client, `c_sdk_player.cpp`) — prediction, view
- `sdk_player_shared.cpp` — movement abilities: dive, slide, wallflip, prone, slowmo/reflexes

`sdk_shareddefs.cpp/.h` holds networked convar and shared constant definitions used across all three contexts.

### Bots (`game/server/sdk/bots/`)

`sdk_bot.cpp`, `sdk_bot_combat.cpp`, `sdk_bot_navigation.cpp` — bot AI. Separate from the player but inherits `CSDKPlayer`.

### Assets and scripts

Game configuration (weapon stats, HUD animations, sound manifests) lives in `mp/game/dab/scripts/` as VDF (Valve Data Format) text files. Localization strings are in `mp/game/dab/resource/dab_english.txt` (and other language variants).

## Code Conventions

Follow Valve's Hungarian notation strictly:
- Member variables: `m_flName` (float), `m_iName` (int), `m_bName` (bool), `m_hName` (handle), `m_pName` (pointer), `m_szName` (string)
- Functions: `CamelCase()`
- Locals and parameters: `flName`, `iName`, etc.

Networked variables use `DECLARE_NETWORKCLASS()` / `CNetworkVar(type, m_varName)` macros. When adding a new networked variable in shared code, it must be declared in both the proxy class and the rules/player class.
