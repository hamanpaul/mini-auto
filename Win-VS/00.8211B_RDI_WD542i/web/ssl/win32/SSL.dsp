# Microsoft Developer Studio Project File - Name="SSL" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=SSL - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SSL.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SSL.mak" CFG="SSL - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SSL - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "SSL - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SSL - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "../inc" /I "../H" /I "./osdl/inc" /I "../../Crypto/inc" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "SSL - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "../inc" /I "../H" /I "./osdl" /I "../../Crypto/common/inc" /I "../../Crypto/RSA/inc" /I "../../Crypto/BN/inc" /I "../../Crypto/Rand/inc" /I "../../Crypto/Cert/inc" /I "../../Crypto/DSS/inc" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\HTTPS\win32\SSL.lib"

!ENDIF 

# Begin Target

# Name "SSL - Win32 Release"
# Name "SSL - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\SSLAlgorithms.c
# End Source File
# Begin Source File

SOURCE=..\src\SSLBuffer.c
# End Source File
# Begin Source File

SOURCE=..\src\SSLHShake.c
# End Source File
# Begin Source File

SOURCE=..\src\SSLKeyEx.c
# End Source File
# Begin Source File

SOURCE=..\src\SSLPort.c
# End Source File
# Begin Source File

SOURCE=..\src\SSLRecord.c
# End Source File
# Begin Source File

SOURCE=..\src\SSLSession.c
# End Source File
# Begin Source File

SOURCE=..\src\SSLUtils.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\inc\SSL.h
# End Source File
# Begin Source File

SOURCE=..\H\SSLAlgorithms.h
# End Source File
# Begin Source File

SOURCE=..\inc\SSLCfg.h
# End Source File
# Begin Source File

SOURCE=..\H\SSLCommon.h
# End Source File
# Begin Source File

SOURCE=..\H\SSLInternal.h
# End Source File
# Begin Source File

SOURCE=..\H\SSLPort.h
# End Source File
# End Group
# Begin Group "OSDL"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Osdl\Os_adpt.c
# End Source File
# End Group
# Begin Group "Crypto"

# PROP Default_Filter ""
# Begin Group "RSA"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Crypto\Rsa\src\asn1typ.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\Rsa\src\objdpn.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\Rsa\src\rsa_chk.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\Rsa\src\rsa_eay.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\Rsa\src\rsa_err.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\Rsa\src\rsa_gen.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\Rsa\src\rsa_lib.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\Rsa\src\rsa_none.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\Rsa\src\rsa_null.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\Rsa\src\rsa_oaep.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\Rsa\src\rsa_pk1.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\Rsa\src\rsa_saos.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\Rsa\src\rsa_sign.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\Rsa\src\rsa_ssl.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\Rsa\src\rsasgndpn.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\Rsa\src\x509dep.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\Rsa\src\x_sig.c
# End Source File
# End Group
# Begin Group "BN"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Crypto\bn\src\bn_add.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\bn\src\bn_asm.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\bn\src\bn_blind.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\bn\src\bn_ctx.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\bn\src\bn_div.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\bn\src\bn_err.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\bn\src\bn_exp.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\bn\src\bn_exp2.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\bn\src\bn_gcd.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\bn\src\bn_lib.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\bn\src\bn_mont.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\bn\src\bn_mpi.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\bn\src\bn_mul.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\bn\src\bn_prime.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\bn\src\bn_prime.h
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\bn\src\bn_print.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\bn\src\bn_rand.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\bn\src\bn_recp.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\bn\src\bn_shift.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\bn\src\bn_sqr.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\bn\src\bn_word.c
# End Source File
# End Group
# Begin Group "RAND"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Crypto\Rand\src\md_rand.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\Rand\src\rand_egd.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\Rand\src\rand_err.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\Rand\src\rand_lib.c
# End Source File
# End Group
# Begin Group "Cert"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Crypto\cert\src\certAsn1dcode.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\cert\src\certBldStruc.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\cert\src\certCoding.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\cert\src\certDecoding.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\cert\src\certElement.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\cert\src\certErrors.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\cert\src\certGstr.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\cert\src\certParserAux.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\cert\src\certStructure.c
# End Source File
# End Group
# Begin Group "DSS"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Crypto\Dss\src\dsa_asn1.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\Dss\src\dsa_dh.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\Dss\src\dsa_err.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\Dss\src\dsa_gen.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\Dss\src\dsa_key.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\Dss\src\dsa_lib.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\Dss\src\dsa_ossl.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\Dss\src\dsa_sign.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\Dss\src\dsa_vrf.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\Dss\src\dsagen.c
# End Source File
# End Group
# Begin Group "Common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Crypto\common\src\CryptCmn.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\common\src\DES.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\common\src\DH.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\common\src\MD5.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\common\src\RC4.c
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\common\src\SHA1.c
# End Source File
# End Group
# End Group
# End Target
# End Project
