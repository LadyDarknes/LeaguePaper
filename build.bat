@echo off
cd /d "%~dp0"
cmake -G Ninja -S . -B build
cmake --build build
