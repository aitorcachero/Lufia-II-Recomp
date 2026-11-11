@echo off
setlocal
cd /d "%~dp0"
if exist "build\Release\LufiaIiSNESRecomp.exe" (
    start "" "build\Release\LufiaIiSNESRecomp.exe" --launcher
) else if exist "LufiaIiSNESRecomp.exe" (
    start "" "LufiaIiSNESRecomp.exe" --launcher
) else (
    echo No se encontro LufiaIiSNESRecomp.exe.
    pause
)
exit /b 0
