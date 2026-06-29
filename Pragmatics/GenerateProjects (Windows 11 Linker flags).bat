@echo off
mkdir KarmaLightHouse
cd KarmaLightHouse

cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_EXE_LINKER_FLAGS="/LIBPATH:\"C:/Program Files (x86)/Windows Kits/10/Lib/10.0.26100.0/ucrt/x64\" /LIBPATH:\"C:/Program Files (x86)/Windows Kits/10/Lib/10.0.26100.0/um/x64\"" ..

echo DO YOU WANT TO OPEN THE SOLUTION?
set /p Input=Enter y(Yes) or n(No):
If /I "%Input%"=="y" goto yes
goto no
:yes
start "" Game_Of_Lands.sln
exit
:no
echo Suit yourself! Goodbye!
PAUSE