# qf.mak
# Created by IBM WorkFrame/2 MakeMake at 13:39:35 on 23 Feb 1998
#
# The actions included in this make file are:
#  Compile::C++ Compiler
#  Link::Linker
#  Bind::Resource Bind
#  Compile::Resource Compiler

.SUFFIXES: .cpp .obj .rc .res 

.all: \
    .\qf.exe

.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /O /Gm /Gd /G5 /C %s

{F:\PRACTICE\QuickFlick\SourceQF}.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /O /Gm /Gd /G5 /C %s

.rc.res:
    @echo " Compile::Resource Compiler "
    rc.exe -r %s .\%|fF.RES

{F:\PRACTICE\QuickFlick\SourceQF}.rc.res:
    @echo " Compile::Resource Compiler "
    rc.exe -r %s .\%|fF.RES

.\qf.exe: \
    .\moviefrm.obj \
    .\qf.obj \
    .\mfcsthlr.obj \
    .\qf.res \
    {$(LIB)}PROCL.LIB \
    {$(LIB)}OS2386.LIB \
    {$(LIB)}MMPM2.LIB \
    {$(LIB)}QUIKFLIK.LIB \
    {$(LIB)}QMMOOV.LIB \
    {$(LIB)}QF.DEF
    @echo " Link::Linker "
    @echo " Bind::Resource Bind "
    icc.exe @<<
     /B" /exepack:2 /pmtype:pm /packd /optfunc /noe /nod"
     /Feqf.exe 
     PROCL.LIB 
     OS2386.LIB 
     MMPM2.LIB 
     QUIKFLIK.LIB 
     QMMOOV.LIB 
     QF.DEF
     .\moviefrm.obj
     .\qf.obj
     .\mfcsthlr.obj
<<
    rc.exe .\qf.res qf.exe

.\moviefrm.obj: \
    F:\PRACTICE\QuickFlick\SourceQF\moviefrm.cpp \
    {F:\PRACTICE\QuickFlick\SourceQF;$(INCLUDE);}moviefrm.hpp \
    {F:\PRACTICE\QuickFlick\SourceQF;$(INCLUDE);}qf.h \
    {F:\PRACTICE\QuickFlick\SourceQF;$(INCLUDE);}mfcsthlr.hpp \
    {F:\PRACTICE\QuickFlick\SourceQF;$(INCLUDE);}quikflik.hpp \
    {F:\PRACTICE\QuickFlick\SourceQF;$(INCLUDE);}quikflik.h \
    {F:\PRACTICE\QuickFlick\SourceQF;$(INCLUDE);}moov.hpp \
    {F:\PRACTICE\QuickFlick\SourceQF;$(INCLUDE);}qmmoov.h

.\mfcsthlr.obj: \
    F:\PRACTICE\QuickFlick\SourceQF\mfcsthlr.cpp \
    {F:\PRACTICE\QuickFlick\SourceQF;$(INCLUDE);}qf.h \
    {F:\PRACTICE\QuickFlick\SourceQF;$(INCLUDE);}mfcsthlr.hpp

.\qf.obj: \
    F:\PRACTICE\QuickFlick\SourceQF\qf.cpp \
    {F:\PRACTICE\QuickFlick\SourceQF;$(INCLUDE);}moviefrm.hpp \
    {F:\PRACTICE\QuickFlick\SourceQF;$(INCLUDE);}qf.h \
    {F:\PRACTICE\QuickFlick\SourceQF;$(INCLUDE);}mfcsthlr.hpp \
    {F:\PRACTICE\QuickFlick\SourceQF;$(INCLUDE);}quikflik.hpp \
    {F:\PRACTICE\QuickFlick\SourceQF;$(INCLUDE);}quikflik.h

.\qf.res: \
    F:\PRACTICE\QuickFlick\SourceQF\qf.rc \
    {F:\PRACTICE\QuickFlick\SourceQF;$(INCLUDE)}QUICKMO.ICO \
    {F:\PRACTICE\QuickFlick\SourceQF;$(INCLUDE)}qf.h
