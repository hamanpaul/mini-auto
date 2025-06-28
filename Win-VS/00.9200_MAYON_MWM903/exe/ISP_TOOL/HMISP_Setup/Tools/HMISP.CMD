@echo off

@MD amd64
@MD x86
@MD ia64
@COPY /Y libusb0.sys_amd64	amd64\libusb0.sys
@COPY /Y libusb0.dll_amd64	amd64\libusb0.dll
@COPY /Y libusb0.sys_x86	x86\libusb0.sys
@COPY /Y libusb0_x86.dll_x86	x86\libusb0_x86.dll
@COPY /Y libusb0.sys_ia64	ia64\libusb0.sys
@COPY /Y libusb0.dll_ia64	ia64\libusb0.dll
@CLS
@ECHO Installing...


if /I [%PROCESSOR_ARCHITECTURE%]==[x86] goto DPINST_X86

if /I [%PROCESSOR_ARCHITECTURE%]==[AMD64] goto DPINST_X64

:DPINST_X86
if defined PROCESSOR_ARCHITEW6432 goto DPINST_X64
call dpinstX86.exe
goto END

:DPINST_X64
call dpinstX64.exe
goto END

:END

