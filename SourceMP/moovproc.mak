# moovproc.mak
# Created by IBM WorkFrame/2 MakeMake at 12:05:37 on 23 Feb 1998
#
# The actions included in this make file are:
#  Compile::C++ Compiler
#  Link::Linker
#  Bind::Resource Bind
#  Compile::Resource Compiler

.SUFFIXES: .cpp .obj .rc .res 

.all: \
    .\moovproc.dll

.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /O /Gm /Gd /Ge- /G5 /C %s

{F:\PRACTICE\QUICKMO\SourceMP}.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /O /Gm /Gd /Ge- /G5 /C %s

.rc.res:
    @echo " Compile::Resource Compiler "
    rc.exe -r %s .\%|fF.RES

{F:\PRACTICE\QUICKMO\SourceMP}.rc.res:
    @echo " Compile::Resource Compiler "
    rc.exe -r %s .\%|fF.RES

.\moovproc.dll: \
    .\moviefil.obj \
    .\moviefmt.obj \
    .\moovfile.obj \
    .\moovfmt.obj \
    .\moovproc.obj \
    .\moovproc.res \
    {$(LIB)}OS2386.LIB \
    {$(LIB)}PROCL.LIB \
    {$(LIB)}MMPM2.LIB \
    {$(LIB)}QMMOOV.LIB \
    {$(LIB)}QUICKMO.LIB \
    {$(LIB)}IMA4.LIB \
    {$(LIB)}MOOVPROC.DEF
    @echo " Link::Linker "
    @echo " Bind::Resource Bind "
    icc.exe @<<
    /DDEBUGGING_IOPROC
     /Tdp 
     /O /Gm /Gd /Ge- /G5 
     /B" /exepack:2 /packd /optfunc /noe"
     /Femoovproc.dll 
     OS2386.LIB 
     PROCL.LIB 
     MMPM2.LIB 
     QMMOOV.LIB 
     QUICKMO.LIB 
     IMA4.LIB 
     MOOVPROC.DEF
     .\moviefil.obj
     .\moviefmt.obj
     .\moovfile.obj
     .\moovfmt.obj
     .\moovproc.obj
<<
    rc.exe .\moovproc.res moovproc.dll

.\moviefil.obj: \
    F:\PRACTICE\QUICKMO\SourceMP\moviefil.cpp \
    {F:\PRACTICE\QUICKMO\SourceMP;$(INCLUDE);}moovproc.h \
    {F:\PRACTICE\QUICKMO\SourceMP;$(INCLUDE);}moviefil.hpp

.\moovproc.obj: \
    F:\PRACTICE\QUICKMO\SourceMP\moovproc.cpp \
    {F:\PRACTICE\QUICKMO\SourceMP;$(INCLUDE);}moovproc.h \
    {F:\PRACTICE\QUICKMO\SourceMP;$(INCLUDE);}moviefmt.hpp \
    {F:\PRACTICE\QUICKMO\SourceMP;$(INCLUDE);}moovfile.hpp \
    {F:\PRACTICE\QUICKMO\SourceMP;$(INCLUDE);}moovfmt.hpp \
    {F:\PRACTICE\QUICKMO\SourceMP;$(INCLUDE);}moviefil.hpp \
    {F:\PRACTICE\QUICKMO\SourceMP;$(INCLUDE);}moov.hpp \
    {F:\PRACTICE\QUICKMO\SourceMP;$(INCLUDE);}qmmoov.h

.\moovfmt.obj: \
    F:\PRACTICE\QUICKMO\SourceMP\moovfmt.cpp \
    {F:\PRACTICE\QUICKMO\SourceMP;$(INCLUDE);}moovproc.h \
    {F:\PRACTICE\QUICKMO\SourceMP;$(INCLUDE);}moviefmt.hpp \
    {F:\PRACTICE\QUICKMO\SourceMP;$(INCLUDE);}moovfmt.hpp \
    {F:\PRACTICE\QUICKMO\SourceMP;$(INCLUDE);}moov.hpp \
    {F:\PRACTICE\QUICKMO\SourceMP;$(INCLUDE);}qmmoov.h

.\moovfile.obj: \
    F:\PRACTICE\QUICKMO\SourceMP\moovfile.cpp \
    {F:\PRACTICE\QUICKMO\SourceMP;$(INCLUDE);}moovproc.h \
    {F:\PRACTICE\QUICKMO\SourceMP;$(INCLUDE);}moviefmt.hpp \
    {F:\PRACTICE\QUICKMO\SourceMP;$(INCLUDE);}moovfile.hpp \
    {F:\PRACTICE\QUICKMO\SourceMP;$(INCLUDE);}moovfmt.hpp \
    {F:\PRACTICE\QUICKMO\SourceMP;$(INCLUDE);}moviefil.hpp \
    {F:\PRACTICE\QUICKMO\SourceMP;$(INCLUDE);}moov.hpp \
    {F:\PRACTICE\QUICKMO\SourceMP;$(INCLUDE);}qmmoov.h

.\moviefmt.obj: \
    F:\PRACTICE\QUICKMO\SourceMP\moviefmt.cpp \
    {F:\PRACTICE\QUICKMO\SourceMP;$(INCLUDE);}moovproc.h \
    {F:\PRACTICE\QUICKMO\SourceMP;$(INCLUDE);}moviefmt.hpp

.\moovproc.res: \
    F:\PRACTICE\QUICKMO\SourceMP\moovproc.rc \
    {F:\PRACTICE\QUICKMO\SourceMP;$(INCLUDE)}QUICKMO.ICO \
    {F:\PRACTICE\QUICKMO\SourceMP;$(INCLUDE)}moovproc.h
