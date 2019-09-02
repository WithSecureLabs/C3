set DIR=%CD%
FOR /d /r . %%d IN (.vs) DO @IF EXIST "%%d" rd /s /q "%%d"
FOR /d /r . %%d IN (Tmp) DO @IF EXIST "%%d" rd /s /q "%%d"
del /s *.obj
del /s *.o
del /s *.pch
del /s *.ipch
del /s *.lastbuildstate
del /s *.tlog
del /s *.pdb
del /s *.ilk
del /s *.idb
del /s *.gch
del /s *.suo
del /s *.VC.db
for /f "usebackq delims=" %%d in (`"dir /ad/b/s | sort /R"`) do rd "%%d"
