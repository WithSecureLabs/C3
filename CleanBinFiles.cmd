FOR /d /r . %%d IN (Bin) DO @IF EXIST "%%d" rd /s /q "%%d"
