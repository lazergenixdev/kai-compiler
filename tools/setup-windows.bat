@echo off
cd ..
cmake -S . -B build
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%
cmake --open build