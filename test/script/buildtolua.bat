@echo off

echo checking %~dp0interface.pkg
cd %~dp0
%~dp0tolua++.exe -t -n interface -o interface.cpp -H interface.h %~dp0interface.pkg
if not "%errorlevel%" == "0" (
            echo error: %~dp0interface.pkg
            goto Failed
            )

goto End

:Failed
pause

:End