#!/usr/bin/env python3
"""Offline integrity checks for the Click Before Frames source bundle."""
import json
import re
import struct
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def png_size(path: Path):
    data = path.read_bytes()
    assert data[:8] == b"\x89PNG\r\n\x1a\n", f"not a PNG: {path}"
    return struct.unpack(">II", data[16:24])


manifest = json.loads((ROOT / "mod.json").read_text())
assert manifest["id"] == "netermindteam.clickbeforeframes"
assert manifest["geode"] == "5.9.0"
assert manifest["gd"].get("android") == "2.2081"
assert "version" not in manifest["gd"]
assert manifest["dependencies"]["geode.node-ids"] == ">=v1.23.3"
assert manifest["version"].startswith("v")

cmake = (ROOT / "CMakeLists.txt").read_text()
assert "CMAKE_CXX_STANDARD 23" in cmake
for source in ("src/main.cpp", "src/CBFPopup.cpp", "src/CBFConfig.cpp", "src/CBFInputEngine.cpp"):
    assert source in cmake, f"source not included in CMake: {source}"

popup_hpp = (ROOT / "src/CBFPopup.hpp").read_text()
popup_cpp = (ROOT / "src/CBFPopup.cpp").read_text()
config_cpp = (ROOT / "src/CBFConfig.cpp").read_text()
main_cpp = (ROOT / "src/main.cpp").read_text()
engine_cpp = (ROOT / "src/CBFInputEngine.cpp").read_text()
assert "public geode::Popup" in popup_hpp
assert "Popup::init(kPopupWidth, kPopupHeight)" in popup_cpp
assert "Popup<>" not in popup_hpp + popup_cpp
assert "initAnchored" not in popup_hpp + popup_cpp
for page in ("buildBufferPage", "buildMovementPage", "buildLevelMemoryPage", "buildPracticePage", "buildDiagnosticsPage", "buildSettingsPage"):
    assert page in popup_hpp and page in popup_cpp, f"missing page: {page}"
assert "right-button-menu" in main_cpp
assert "setupHasCompleted" in main_cpp
assert "bool pushButton(PlayerButton button)" in main_cpp
assert "bool releaseButton(PlayerButton button)" in main_cpp
assert "void processCommands(float dt, bool isHalfTick, bool isLastTick)" in main_cpp
assert "advanceGameStep" in main_cpp
assert "delayForPress" in engine_cpp
assert "levelMemory()" in engine_cpp
assert "click-memory-v1" in engine_cpp
assert "event.player->pushButton(event.button)" in engine_cpp
assert "event.player->releaseButton(event.button)" in engine_cpp
assert "m_isDart" in engine_cpp
assert "level-{}-local-{:016x}" in config_cpp
assert "std::clamp(read<int>(\"wave-buffer-frames\", 5), 0, 20)" in config_cpp

logo_size = png_size(ROOT / "logo.png")
button_size = png_size(ROOT / "resources" / "cbf-button.png")
menu_size = png_size(ROOT / "reference" / "menu-style.png")
assert logo_size == (512, 512), logo_size
assert button_size == (512, 512), button_size
assert menu_size == (1672, 941), menu_size

workflow = (ROOT / ".github" / "workflows" / "build-android.yml").read_text()
assert "prevter/geode-sdk:android-5.9.0" in workflow
assert "geode build -p android64" in workflow

print("PASS: mod.json schema and identifiers")
print("PASS: CMake C++23 source list")
print("PASS: Geode v5 Popup API migration")
print("PASS: six independent page builders and level hooks")
print("PASS: per-level settings identity and frame bounds")
print("PASS: press/release interception and simulation-step delay queue")
print("PASS: per-level click-position memory persistence")
print("PASS: icon/menu assets and pinned Android toolchain")
print("NOTE: this script does not compile Geode C++ or test the game runtime")
