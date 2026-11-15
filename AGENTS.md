# AGENTS.md — XZG

Guide for AI agents and humans. Keep it current when you change build, env or structure.

## What this is
XZG — Zigbee/serial gateway firmware for ESP32 boards with a TI CC26xx/CC2652 radio (UZG-01, SLZB-06, ZigStar,
Olimex, …). Arduino framework, PlatformIO. Upstream: `git@github.com:xyzroe/XZG.git` (branch `main`).
Sibling repo `../XZG_esp-idf` is a local-only plain ESP-IDF rewrite (see its `AGENTS.md`, `docs/PORT_SCOPE.md`).

## Environment (macOS, Apple Silicon)
- Everything must run **native arm64**. `/usr/local/bin` is Intel Homebrew (x86_64); avoid tools from there for new installs.
- PlatformIO: use `~/.platformio/penv/bin/pio` (arm64). `/usr/local/bin/pio` is broken (points to python2.7).
- Node for the web build: nvm `v18.17.0` (CI uses 18) or `v20.18.3` — both arm64; other nvm versions are x86_64.
  `export PATH=$HOME/.nvm/versions/node/v18.17.0/bin:$PATH`
- Submodule `lib/WireGuard` is required: `git submodule update --init --recursive`.

## Build
CI (`.github/workflows/build_fw.yml` on tag, `test_build_fw.yml` on PR) = Python 3.12, Node 18,
`pip install platformio==6.2.0 && pio run` (default env `prod-dual`). Releases: push a tag `YYYYMMDD`; the workflow
builds, publishes a **pre-release** (notes = body of the last commit message), copies full.bin to the `releases`
branch and updates `releases/latest/xzg.json` (devices' update check), then notifies Telegram/Discord.
Releases must be built by Actions only. Local equivalent of the build:
```bash
export PATH=$HOME/.nvm/versions/node/v18.17.0/bin:$PATH
~/.platformio/penv/bin/pio run -e prod-dual
```
Pipeline (extra_scripts in `platformio.ini`):
1. `tools/build/pre_build.py` — generates `src/const/keys.h` from `keys.cpp`; defines `PLATFORM_NAME` ("espressif32@x.y.z").
2. `tools/build/version_update.py` — writes `src/version.h` (`YYYYMMDD`; CI overwrites with the tag). Don't commit build-date bumps.
3. `tools/webfilesbuilder/build_html.py` — **full build**: npm install (once) → `npx gulp xzg` (minify+gzip `src/websrc` →
   `data/`), writes `data/x/commit` (last commit of `src/websrc`), builds LittleFS image → `bin/XZG.fs.bin`.
   **Short build** (code only): `touch tools/.no_web_update` → web/FS step skipped.
4. `tools/build/build.py` (post) — full build: merges bootloader+partitions+app+fs into
   `bin/XZG_<ver>.full.bin`, plus `.ota.bin` and `.fs.bin`. Short build: only `.ota.bin`.
   SCons skips relinking when nothing changed — delete `.pio/build/<env>/firmware.*` to force outputs.
- Envs: `prod-dual` (esp32dev, espressif32 6.10.0 = Arduino 2.0.17 / IDF 4.4), `debug-dual` (+`-DDEBUG`,
  `DISABLE_FS_AUTO_UPDATE`), `prod-solo`/`debug-solo` (Tasmota platform, Arduino 3.1.3 / IDF 5.3).
- App flash usage is ~98.4% of the 1.25 MB slot — watch size when adding code/libs.
- Known: `prod-solo` fails locally at CMake (`git config --get remote.origin.url` in IDF component manager for a git
  dependency of the Tasmota framework). Not built by CI.

## Flash & verify
See TESTBENCH.md. Short version:
```bash
ESPTOOL="$HOME/.platformio/penv/bin/python $HOME/.platformio/packages/tool-esptoolpy/esptool.py"
$ESPTOOL --chip esp32 -p <port> -b 460800 write_flash 0x0 bin/XZG_<ver>.full.bin   # erase_flash first to force board autodetect
~/.platformio/penv/bin/python ../XZG_esp-idf/tools/test/serial_log.py <alias> --reset -t 60
../XZG_esp-idf/tools/test/check_device.sh <alias>
```
Board name is stored in NVS; autodetect (`findBrdConfig`, chain `iNN` → `Try brd` → reboot …) only runs with empty NVS.

## Layout
- `src/main.cpp` — setup/loop, network overseer (Ticker), socket server :6638 (Zigbee serial over TCP), first update check.
- `src/web.cpp` — WebServer, `/api?action=` (pages, params, ~30 commands), SSE events, OTA upload, update fetch.
- `src/zb.cpp` + `lib/CCTools` — CC26xx BSL: detect, flash from URL, NV erase, LED, router rejoin.
- `src/mqtt.cpp` — AsyncMqttClient + Home Assistant discovery. `src/etc.cpp` — misc: time/cron, board autodetect, WireGuard,
  DNS, update checks, 1-Wire DS18B20. `src/config.cpp` — NVS config structs ↔ JSON. `src/per.cpp` — button/LEDs.
- `src/const/hw.cpp` — board tables (`brdConfigs`, `ethConfigs`, `zbConfigs`, `mistConfigs`). `src/websrc/` — web UI sources.
- `bin/` — tracked `bootloader_dio_40m.bin`, `partitions.bin`; build outputs are ignored.

## Notes / pitfalls
- Never block the Ticker/esp_timer callbacks (`handleTmrNetworkOverseer`); do long work from `loop()` or a task.
- LittleFS is not safe for concurrent file access from different tasks on this core (seen as `assert lfs_file_seek`) —
  keep FS-using code (update checks, web handlers) on the loop task.
- HTTPS update checks need valid time: the first update check waits for NTP (up to 5 min) before running.
