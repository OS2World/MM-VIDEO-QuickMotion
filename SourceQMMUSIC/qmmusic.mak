# qmmusic.mak
# Created by IBM WorkFrame/2 MakeMake at 16:08:01 on 20 Feb 1998
#
# The actions included in this make file are:
#  Compile::C++ Compiler
#  Link::Linker
#  Bind::Resource Bind
#  Compile::Resource Compiler

.SUFFIXES: .cpp .obj .rc .res 

.all: \
    .\qmmusic.dll

.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /O /Gm /Gd /Ge- /G5 /Gn /C %s

{F:\PRACTICE\QUICKMO\SourceQMMUSIC}.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /O /Gm /Gd /Ge- /G5 /Gn /C %s

.rc.res:
    @echo " Compile::Resource Compiler "
    rc.exe -r %s .\%|fF.RES

{F:\PRACTICE\QUICKMO\SourceQMMUSIC}.rc.res:
    @echo " Compile::Resource Compiler "
    rc.exe -r %s .\%|fF.RES

.\qmmusic.dll: \
    .\musifmt.obj \
    .\qmmusic.obj \
    .\midifile.obj \
    .\midifmt.obj \
    .\musifile.obj \
    .\qmmusic.res \
    {$(LIB)}OS2386.LIB \
    {$(LIB)}PROCL.LIB \
    {$(LIB)}MMPM2.LIB \
    {$(LIB)}QMMOOV.LIB \
    {$(LIB)}QMMUSIC.DEF
    @echo " Link::Linker "
    @echo " Bind::Resource Bind "
    icc.exe @<<
    /Tdp 
     /O /Gm /Gd /Ge- /G5 /Gn 
     /B" /exepack:2 /packd /optfunc /noe /nod"
     /Feqmmusic.dll 
     OS2386.LIB 
     PROCL.LIB 
     MMPM2.LIB 
     QMMOOV.LIB 
     QMMUSIC.DEF
     .\musifmt.obj
     .\qmmusic.obj
     .\midifile.obj
     .\midifmt.obj
     .\musifile.obj
<<
    rc.exe .\qmmusic.res qmmusic.dll

.\musifmt.obj: \
    F:\PRACTICE\QUICKMO\SourceQMMUSIC\musifmt.cpp \
    {F:\PRACTICE\QUICKMO\SourceQMMUSIC;$(INCLUDE);}qmmusic.h \
    {F:\PRACTICE\QUICKMO\SourceQMMUSIC;$(INCLUDE);}musifmt.hpp \
    {F:\PRACTICE\QUICKMO\SourceQMMUSIC;$(INCLUDE);}midifmt.hpp \
    {F:\PRACTICE\QUICKMO\SourceQMMUSIC;$(INCLUDE);}moov.hpp \
    {F:\PRACTICE\QUICKMO\SourceQMMUSIC;$(INCLUDE);}qmmoov.h

.\musifile.obj: \
    F:\PRACTICE\QUICKMO\SourceQMMUSIC\musifile.cpp \
    {F:\PRACTICE\QUICKMO\SourceQMMUSIC;$(INCLUDE);}qmmusic.h \
    {F:\PRACTICE\QUICKMO\SourceQMMUSIC;$(INCLUDE);}musifmt.hpp \
    {F:\PRACTICE\QUICKMO\SourceQMMUSIC;$(INCLUDE);}musifile.hpp \
    {F:\PRACTICE\QUICKMO\SourceQMMUSIC;$(INCLUDE);}midifmt.hpp \
    {F:\PRACTICE\QUICKMO\SourceQMMUSIC;$(INCLUDE);}midifile.hpp \
    {F:\PRACTICE\QUICKMO\SourceQMMUSIC;$(INCLUDE);}moov.hpp \
    {F:\PRACTICE\QUICKMO\SourceQMMUSIC;$(INCLUDE);}qmmoov.h

.\midifmt.obj: \
    F:\PRACTICE\QUICKMO\SourceQMMUSIC\midifmt.cpp \
    {F:\PRACTICE\QUICKMO\SourceQMMUSIC;$(INCLUDE);}qmmusic.h \
    {F:\PRACTICE\QUICKMO\SourceQMMUSIC;$(INCLUDE);}midifmt.hpp

.\midifile.obj: \
    F:\PRACTICE\QUICKMO\SourceQMMUSIC\midifile.cpp \
    {F:\PRACTICE\QUICKMO\SourceQMMUSIC;$(INCLUDE);}qmmusic.h \
    {F:\PRACTICE\QUICKMO\SourceQMMUSIC;$(INCLUDE);}midifile.hpp

.\qmmusic.obj: \
    F:\PRACTICE\QUICKMO\SourceQMMUSIC\qmmusic.cpp \
    {F:\PRACTICE\QUICKMO\SourceQMMUSIC;$(INCLUDE);}qmmusic.h \
    {F:\PRACTICE\QUICKMO\SourceQMMUSIC;$(INCLUDE);}musifmt.hpp \
    {F:\PRACTICE\QUICKMO\SourceQMMUSIC;$(INCLUDE);}musifile.hpp \
    {F:\PRACTICE\QUICKMO\SourceQMMUSIC;$(INCLUDE);}midifmt.hpp \
    {F:\PRACTICE\QUICKMO\SourceQMMUSIC;$(INCLUDE);}midifile.hpp \
    {F:\PRACTICE\QUICKMO\SourceQMMUSIC;$(INCLUDE);}moov.hpp \
    {F:\PRACTICE\QUICKMO\SourceQMMUSIC;$(INCLUDE);}qmmoov.h

.\qmmusic.res: \
    F:\PRACTICE\QUICKMO\SourceQMMUSIC\qmmusic.rc \
    {F:\PRACTICE\QUICKMO\SourceQMMUSIC;$(INCLUDE)}QUICKMO.ICO \
    {F:\PRACTICE\QUICKMO\SourceQMMUSIC;$(INCLUDE)}qmmusic.h
