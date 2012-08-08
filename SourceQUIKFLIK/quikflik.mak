# quikflik.mak
# Created by IBM WorkFrame/2 MakeMake at 14:14:07 on 23 Feb 1998
#
# The actions included in this make file are:
#  Compile::C++ Compiler
#  Link::Linker
#  Bind::Resource Bind
#  Compile::Resource Compiler

.SUFFIXES: .cpp .obj .rc .res 

.all: \
    .\quikflik.dll

.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /O /Gm /Gd /Ge- /G5 /Gf /Gi /C %s

{F:\PRACTICE\QuickFlick\SourceQUIKFLIK}.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /O /Gm /Gd /Ge- /G5 /Gf /Gi /C %s

.rc.res:
    @echo " Compile::Resource Compiler "
    rc.exe -r %s .\%|fF.RES

{F:\PRACTICE\QuickFlick\SourceQUIKFLIK}.rc.res:
    @echo " Compile::Resource Compiler "
    rc.exe -r %s .\%|fF.RES

.\quikflik.dll: \
    F:\PRACTICE\QuickFlick\SourceQUIKFLIK\qfwpanof.obj \
    .\quikflik.obj \
    .\propdiag.obj \
    .\qfcontrol.obj \
    .\qfmotion.obj \
    .\qfmphlr.obj \
    .\qfphlr.obj \
    .\qfplayer.obj \
    .\qfwobjc.obj \
    .\qfworld.obj \
    .\qfwpano.obj \
    .\qfwphlr.obj \
    .\qfwrend.obj \
    .\quikflik.res \
    {$(LIB)}PROCL.LIB \
    {$(LIB)}OS2386.LIB \
    {$(LIB)}MMPM2.LIB \
    {$(LIB)}QMMOOV.LIB \
    {$(LIB)}QFBASE.LIB \
    {$(LIB)}quikflik.def
    @echo " Link::Linker "
    @echo " Bind::Resource Bind "
    icc.exe @<<
    /Tdp 
     /O /Gm /Gd /Ge- /G5 /Gf /Gi 
     /B" /exepack:2 /nobase /nop /packd /optfunc /noe /nod"
     /Fequikflik.dll 
     PROCL.LIB 
     OS2386.LIB 
     MMPM2.LIB 
     QMMOOV.LIB 
     QFBASE.LIB 
     quikflik.def
     F:\PRACTICE\QuickFlick\SourceQUIKFLIK\qfwpanof.obj
     .\quikflik.obj
     .\propdiag.obj
     .\qfcontrol.obj
     .\qfmotion.obj
     .\qfmphlr.obj
     .\qfphlr.obj
     .\qfplayer.obj
     .\qfwobjc.obj
     .\qfworld.obj
     .\qfwpano.obj
     .\qfwphlr.obj
     .\qfwrend.obj
<<
    rc.exe .\quikflik.res quikflik.dll

.\qfwrend.obj: \
    F:\PRACTICE\QuickFlick\SourceQUIKFLIK\qfwrend.cpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}moov.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}quikflik.h \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qmmoov.h \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfwrend.hpp

.\qfwphlr.obj: \
    F:\PRACTICE\QuickFlick\SourceQUIKFLIK\qfwphlr.cpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}quikflik.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfworld.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}quikflik.h \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfplayer.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfwphlr.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfphlr.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfwrend.hpp

.\qfwpano.obj: \
    F:\PRACTICE\QuickFlick\SourceQUIKFLIK\qfwpano.cpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}quikflik.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfworld.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}moov.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}quikflik.h \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfplayer.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfwphlr.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qmmoov.h \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfphlr.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}pluginio.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfwrend.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfwpano.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfwpanof.hpp

.\qfworld.obj: \
    F:\PRACTICE\QuickFlick\SourceQUIKFLIK\qfworld.cpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}quikflik.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfworld.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}moov.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}quikflik.h \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfplayer.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfwphlr.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qmmoov.h \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfphlr.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfwobjc.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfwrend.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfwpano.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfwpanof.hpp

.\qfwobjc.obj: \
    F:\PRACTICE\QuickFlick\SourceQUIKFLIK\qfwobjc.cpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfworld.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}moov.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}quikflik.h \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfplayer.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfwphlr.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qmmoov.h \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfphlr.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfwobjc.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}pluginio.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfwrend.hpp

.\qfplayer.obj: \
    F:\PRACTICE\QuickFlick\SourceQUIKFLIK\qfplayer.cpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}quikflik.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}quikflik.h \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfplayer.hpp

.\qfphlr.obj: \
    F:\PRACTICE\QuickFlick\SourceQUIKFLIK\qfphlr.cpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}quikflik.h \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfphlr.hpp

.\qfmphlr.obj: \
    F:\PRACTICE\QuickFlick\SourceQUIKFLIK\qfmphlr.cpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}quikflik.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfmotion.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}quikflik.h \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}IMMDEV.HPP \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfplayer.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfmphlr.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfphlr.hpp

.\qfmotion.obj: \
    F:\PRACTICE\QuickFlick\SourceQUIKFLIK\qfmotion.cpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}quikflik.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfmotion.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfcontrol.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}quikflik.h \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}IMMDEV.HPP \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfplayer.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfmphlr.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfphlr.hpp

.\qfcontrol.obj: \
    F:\PRACTICE\QuickFlick\SourceQUIKFLIK\qfcontrol.cpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfcontrol.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}quikflik.h

.\propdiag.obj: \
    F:\PRACTICE\QuickFlick\SourceQUIKFLIK\propdiag.cpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}propdiag.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}quikflik.h

.\quikflik.obj: \
    F:\PRACTICE\QuickFlick\SourceQUIKFLIK\quikflik.cpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}quikflik.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfmotion.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfworld.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfcontrol.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}propdiag.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}moov.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}quikflik.h \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}IMMDEV.HPP \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfplayer.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfmphlr.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfwphlr.hpp \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qmmoov.h \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE);}qfphlr.hpp

.\quikflik.res: \
    F:\PRACTICE\QuickFlick\SourceQUIKFLIK\quikflik.rc \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE)}HOTHAND.PTR \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE)}ZOOM.PTR \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE)}SPINHAND.PTR \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE)}SPINARRO.PTR \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE)}PULLHAND.PTR \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE)}OPENHAND.PTR \
    {F:\PRACTICE\QuickFlick\SourceQUIKFLIK;$(INCLUDE)}quikflik.h
