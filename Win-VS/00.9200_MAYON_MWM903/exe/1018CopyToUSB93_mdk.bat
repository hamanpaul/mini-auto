cd exe

del .\BL1016.bin /q /f
copy BL18UB93.bin BL1016.bin

start GenerateFile.exe
