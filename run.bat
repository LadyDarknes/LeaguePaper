@echo off
cd /d "%~dp0"
call build.bat
cls
build\league-paper.exe
