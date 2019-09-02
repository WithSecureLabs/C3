@echo off
cd WebController
if "%1"=="" (
	set tmp="http://localhost:52935"
) else ( 
	set tmp=%1
)
dotnet C3WebController.dll --urls %tmp%
