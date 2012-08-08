# qmsound.mak
# Created by IBM WorkFrame/2 MakeMake at 16:11:32 on 20 Feb 1998
#
# The actions included in this make file are:
#  Compile::C++ Compiler
#  Link::Linker
#  Bind::Resource Bind
#  Compile::Resource Compiler

.SUFFIXES: .cpp .obj .rc .res 

.all: \
    .\qmsound.dll

.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /O /Gm /Gd /Ge- /G5 /Gn /C %s

{F:\PRACTICE\QUICKMO\SourceQMSOUND}.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /O /Gm /Gd /Ge- /G5 /Gn /C %s

.rc.res:
    @echo " Compile::Resource Compiler "
    rc.exe -r %s .\%|fF.RES

{F:\PRACTICE\QUICKMO\SourceQMSOUND}.rc.res:
    @echo " Compile::Resource Compiler "
    rc.exe -r %s .\%|fF.RES

.\qmsound.dll: \
    .\soundfil.obj \
    .\soundfmt.obj \
    .\audiofil.obj \
    .\audiofmt.obj \
    .\qmsound.obj \
    .\qmsound.res \
    {$(LIB)}OS2386.LIB \
    {$(LIB)}PROCL.LIB \
    {$(LIB)}MMPM2.LIB \
    {$(LIB)}QMMOOV.LIB \
    {$(LIB)}IMA4.LIB \
    {$(LIB)}QMSOUND.DEF
    @echo " Link::Linker "
    @echo " Bind::Resource Bind "
    icc.exe @<<
    /Tdp 
     /O /Gm /Gd /Ge- /G5 /Gn 
     /B" /exepack:2 /packd /optfunc /noe /nod"
     /Feqmsound.dll 
     OS2386.LIB 
     PROCL.LIB 
     MMPM2.LIB 
     QMMOOV.LIB 
     IMA4.LIB 
     QMSOUND.DEF
     .\soundfil.obj
     .\soundfmt.obj
     .\audiofil.obj
     .\audiofmt.obj
     .\qmsound.obj
<<
    rc.exe .\qmsound.res qmsound.dll

.\soundfil.obj: \
    F:\PRACTICE\QUICKMO\SourceQMSOUND\soundfil.cpp \
    {F:\PRACTICE\QUICKMO\SourceQMSOUND;$(INCLUDE);}qmsound.h \
    {F:\PRACTICE\QUICKMO\SourceQMSOUND;$(INCLUDE);}soundfmt.hpp \
    {F:\PRACTICE\QUICKMO\SourceQMSOUND;$(INCLUDE);}moov.hpp \
    {F:\PRACTICE\QUICKMO\SourceQMSOUND;$(INCLUDE);}audiofmt.hpp \
    {F:\PRACTICE\QUICKMO\SourceQMSOUND;$(INCLUDE);}qmmoov.h \
    {F:\PRACTICE\QUICKMO\SourceQMSOUND;$(INCLUDE);}audiofil.hpp \
    {F:\PRACTICE\QUICKMO\SourceQMSOUND;$(INCLUDE);}soundfil.hpp

.\qmsound.obj: \
    F:\PRACTICE\QUICKMO\SourceQMSOUND\qmsound.cpp \
    {F:\PRACTICE\QUICKMO\SourceQMSOUND;$(INCLUDE);}qmsound.h \
    {F:\PRACTICE\QUICKMO\SourceQMSOUND;$(INCLUDE);}soundfmt.hpp \
    {F:\PRACTICE\QUICKMO\SourceQMSOUND;$(INCLUDE);}moov.hpp \
    {F:\PRACTICE\QUICKMO\SourceQMSOUND;$(INCLUDE);}audiofmt.hpp \
    {F:\PRACTICE\QUICKMO\SourceQMSOUND;$(INCLUDE);}qmmoov.h \
    {F:\PRACTICE\QUICKMO\SourceQMSOUND;$(INCLUDE);}audiofil.hpp \
    {F:\PRACTICE\QUICKMO\SourceQMSOUND;$(INCLUDE);}soundfil.hpp

.\audiofmt.obj: \
    F:\PRACTICE\QUICKMO\SourceQMSOUND\audiofmt.cpp \
    {F:\PRACTICE\QUICKMO\SourceQMSOUND;$(INCLUDE);}qmsound.h \
    {F:\PRACTICE\QUICKMO\SourceQMSOUND;$(INCLUDE);}audiofmt.hpp

.\audiofil.obj: \
    F:\PRACTICE\QUICKMO\SourceQMSOUND\audiofil.cpp \
    {F:\PRACTICE\QUICKMO\SourceQMSOUND;$(INCLUDE);}qmsound.h \
    {F:\PRACTICE\QUICKMO\SourceQMSOUND;$(INCLUDE);}audiofil.hpp

.\soundfmt.obj: \
    F:\PRACTICE\QUICKMO\SourceQMSOUND\soundfmt.cpp \
    {F:\PRACTICE\QUICKMO\SourceQMSOUND;$(INCLUDE);}qmsound.h \
    {F:\PRACTICE\QUICKMO\SourceQMSOUND;$(INCLUDE);}soundfmt.hpp \
    {F:\PRACTICE\QUICKMO\SourceQMSOUND;$(INCLUDE);}moov.hpp \
    {F:\PRACTICE\QUICKMO\SourceQMSOUND;$(INCLUDE);}audiofmt.hpp \
    {F:\PRACTICE\QUICKMO\SourceQMSOUND;$(INCLUDE);}qmmoov.h

.\qmsound.res: \
    F:\PRACTICE\QUICKMO\SourceQMSOUND\qmsound.rc \
    {F:\PRACTICE\QUICKMO\SourceQMSOUND;$(INCLUDE)}QUICKMO.ICO \
    {F:\PRACTICE\QUICKMO\SourceQMSOUND;$(INCLUDE)}qmsound.h
