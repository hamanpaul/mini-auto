@echo off
SET INPUT_DIR=DriverSuite
SET OUTPUT_DIR=Installer

MD ..\%OUTPUT_DIR%
COPY /Y ..\%INPUT_DIR%\HMISP.INF		..\%OUTPUT_DIR%\HMISP.INF
COPY /Y ..\%INPUT_DIR%\amd64\libusb0.sys	..\%OUTPUT_DIR%\libusb0.sys_amd64
COPY /Y ..\%INPUT_DIR%\amd64\libusb0.dll	..\%OUTPUT_DIR%\libusb0.dll_amd64
COPY /Y ..\%INPUT_DIR%\x86\libusb0.sys		..\%OUTPUT_DIR%\libusb0.sys_x86
COPY /Y ..\%INPUT_DIR%\x86\libusb0_x86.dll	..\%OUTPUT_DIR%\libusb0_x86.dll_x86
COPY /Y ..\%INPUT_DIR%\ia64\libusb0.sys		..\%OUTPUT_DIR%\libusb0.sys_ia64
COPY /Y ..\%INPUT_DIR%\ia64\libusb0.dll		..\%OUTPUT_DIR%\libusb0.dll_ia64
COPY /Y HMISP.CMD				..\%OUTPUT_DIR%\HMISP.CMD
COPY /Y dpinst.xml				..\%OUTPUT_DIR%\dpinst.xml
COPY /Y dpinstx86.exe				..\%OUTPUT_DIR%\dpinstx86.exe
COPY /Y dpinstx64.exe				..\%OUTPUT_DIR%\dpinstx64.exe

@echo COMPLETED