# qmmoov.mak
# Created by IBM WorkFrame/2 MakeMake at 16:04:48 on 20 Feb 1998
#
# The actions included in this make file are:
#  Compile::C++ Compiler
#  Link::Linker
#  Bind::Resource Bind
#  Compile::Resource Compiler

.SUFFIXES: .cpp .obj .rc .res 

.all: \
    .\qmmoov.dll

.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /O /Gm /Gd /Ge- /G5 /Gn /C %s

{F:\PRACTICE\QUICKMO\SourceQMMOOV}.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /O /Gm /Gd /Ge- /G5 /Gn /C %s

.rc.res:
    @echo " Compile::Resource Compiler "
    rc.exe -r %s .\%|fF.RES

{F:\PRACTICE\QUICKMO\SourceQMMOOV}.rc.res:
    @echo " Compile::Resource Compiler "
    rc.exe -r %s .\%|fF.RES

.\qmmoov.dll: \
    .\moov.obj \
    .\qmmoov.res \
    {$(LIB)}OS2386.LIB \
    {$(LIB)}PROCL.LIB \
    {$(LIB)}MMPM2.LIB \
    {$(LIB)}QMMOOV.DEF
    @echo " Link::Linker "
    @echo " Bind::Resource Bind "
    icc.exe @<<
    /Tdp 
     /O /Gm /Gd /Ge- /G5 /Gn 
     /B" /exepack:2 /packd /optfunc /noe /nod"
     /Feqmmoov.dll 
     OS2386.LIB 
     PROCL.LIB 
     MMPM2.LIB 
     QMMOOV.DEF
     .\moov.obj
<<
    rc.exe .\qmmoov.res qmmoov.dll

.\moov.obj: \
    F:\PRACTICE\QUICKMO\SourceQMMOOV\moov.cpp \
    {F:\PRACTICE\QUICKMO\SourceQMMOOV;$(INCLUDE);}moov.hpp \
    {F:\PRACTICE\QUICKMO\SourceQMMOOV;$(INCLUDE);}qmmoov.h

.\qmmoov.res: \
    F:\PRACTICE\QUICKMO\SourceQMMOOV\qmmoov.rc \
    {F:\PRACTICE\QUICKMO\SourceQMMOOV;$(INCLUDE)}qmmoov.h
