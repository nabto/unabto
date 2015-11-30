@echo off
setlocal
set PJROOT=..\..\..\..\..\..\..\..\..\..\
set GMAKEARGS=%*
if exist BuildLog.txt del BuildLog.txt
%PJROOT%\Tools\xtee\xtee.exe -at BuildLog.txt %PJROOT%\Tools\RtxBuild\gmake.bat %PJROOT% prepare process
