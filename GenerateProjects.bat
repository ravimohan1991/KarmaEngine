@echo off
mkdir KarmaLightHouse
cd KarmaLightHouse
cmake ../ -DCMAKE_BUILD_TYPE=Debug

echo DO YOU WANT TO OPEN THE SOLUTION?
set /p Input=Enter y(Yes) or n(No):
If /I "%Input%"=="y" goto yes
goto no
:yes
KarmaBasedProjectName.sln
exit
:no
echo Suit yourself! Goodbye!
PAUSE