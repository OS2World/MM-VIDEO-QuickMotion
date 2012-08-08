# qfbase.mak
# Created by IBM WorkFrame/2 MakeMake at 18:03:28 on 24 Feb 1998
#
# The actions included in this make file are:
#  Compile::C++ Compiler
#  Link::Linker
#  Lib::Import Lib

.SUFFIXES: .LIB .cpp .dll .obj 

.all: \
    .\qfbase.LIB

.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /O /Gm /Ge- /G5 /Gn /C %s

{F:\PRACTICE\QuickFlick\SourceQFBASE}.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /O /Gm /Ge- /G5 /Gn /C %s

.dll.LIB:
    @echo " Lib::Import Lib "
    implib.exe %|dpfF.LIB %s

{F:\PRACTICE\QuickFlick\SourceQFBASE}.dll.LIB:
    @echo " Lib::Import Lib "
    implib.exe %|dpfF.LIB %s

.\qfbase.dll: \
    .\qfbase.obj \
    {$(LIB)}PROCL.LIB \
    {$(LIB)}OS2386.LIB \
    {$(LIB)}QFBASE.DEF
    @echo " Link::Linker "
    icc.exe @<<
     /B" /exepack:2 /packd /optfunc /noe /nod"
     /Feqfbase.dll 
     PROCL.LIB 
     OS2386.LIB 
     QFBASE.DEF
     .\qfbase.obj
<<

.\qfbase.obj: \
    F:\PRACTICE\QuickFlick\SourceQFBASE\qfbase.cpp \
    {F:\PRACTICE\QuickFlick\SourceQFBASE;$(INCLUDE);}qfwrend.hpp \
    {F:\PRACTICE\QuickFlick\SourceQFBASE;$(INCLUDE);}quikflik.h

.\qfbase.LIB: \
    .\qfbase.dll
