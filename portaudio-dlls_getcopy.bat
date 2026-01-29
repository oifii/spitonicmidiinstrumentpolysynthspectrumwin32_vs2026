echo off
rem https://stackoverflow.com/questions/138497/iterate-all-files-in-a-directory-using-a-for-loop
rem https://stackoverflow.com/questions/5777400/how-to-use-random-in-batch-script
rem https://stackoverflow.com/questions/7522740/counting-in-a-for-loop-using-windows-batch-script
rem setlocal enableextensions enabledelayedexpansion
rem handle output to stderr - Add the 2>&1 redirection so it is like this: dir 2>&1 > a.txt & type a.txt and you should have stderr mixed in with stdout.

rem 2022oct21
rem spitxtfrombatch.bat (from spiexecutablefrombatch.bat
rem works!
set mynopathfilenameext="%~nx0"
set myext="%~x0"
set myfullpath="%~dp0"
set myfullpathfilenamenoext="%~dpn0"
set myfullpathfilenamenoextnoquotes=%myfullpathfilenamenoext:"=%
rem set executableextnoquotes=.exe
set outputtxtfileextnoquotes=.txt
rem set devicetxtfileextnoquotes=device.txt
rem set fullpathexecutableext="%myfullpathfilenamenoextnoquotes%%executableextnoquotes%" 
set fullpathoutputtxtfilext="%myfullpathfilenamenoextnoquotes%%outputtxtfileextnoquotes%" 
rem set fullpathdevicetxtfilext="%~dp0%devicetxtfileextnoquotes%" 
set mydrive=%~d0
rem echo %mynopathfilenameext%
rem echo %myfullpathfilenamenoext%
rem echo %myfullpath%
rem echo %mydrive%
rem echo %myext%

rem 2022oct23
rem cd %myfullpath%
rem %mydrive%

rem
rem Stream0 is STDIN, i.e dir 0>input.txt 
rem Stream1 is STDOUT, i.e dir 1>output.txt 
rem Stream2 is STDERR, i.e dir 2>error.txt 
rem Stream3-9 are not used
rem
rem copy ..\lib-src\portaudio\build\msvc\Win32\Debug\portaudio_x86.dll .\portaudio_x86.dll
rem copy ..\lib-src\portaudio\build\msvc\Win32\Debug\portaudio_x86.dll .\Debug\portaudio_x86.dll
rem copy ..\lib-src\portaudio\build\msvc\Win32\Release\portaudio_x86.dll .\Release\portaudio_x86.dll
rem copy ..\lib-src\portaudio(x64)\build\msvc\x64\Debug\portaudio_x64.dll .\x64\Debug\portaudio_x64.dll
rem copy ..\lib-src\portaudio(x64)\build\msvc\x64\Release\portaudio_x64.dll .\x64\Release\portaudio_x64.dll

rem
set LOGFILE=%fullpathoutputtxtfilext%
call :executeandlog > %LOGFILE%
exit /B

:executeandlog
echo on
@rem x86
copy ..\lib-src\portaudio-2021\portaudio_vs2026\build\msvc\Win32\Debug\portaudio_x86.dll .\portaudio_x86.dll
copy ..\lib-src\portaudio-2021\portaudio_vs2026\build\msvc\Win32\Debug\portaudio_x86.dll .\Debug\portaudio_x86.dll
copy ..\lib-src\portaudio-2021\portaudio_vs2026\build\msvc\Win32\Release\portaudio_x86.dll .\Release\portaudio_x86.dll
@rem x64
copy ..\lib-src\portaudio-2021\portaudio_vs2026\build\msvc\x64\Debug\portaudio_x64.dll .\portaudio_x64.dll
copy ..\lib-src\portaudio-2021\portaudio_vs2026\build\msvc\x64\Debug\portaudio_x64.dll .\x64\Debug\portaudio_x64.dll
copy ..\lib-src\portaudio-2021\portaudio_vs2026\build\msvc\x64\Release\portaudio_x64.dll .\x64\Release\portaudio_x64.dll

