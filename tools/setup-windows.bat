@ECHO ON
CD ..
cmake -S . -B build
:: IF %ERRORLEVEL% neq 0 EXIT /b %ERRORLEVEL%
cmake --open build