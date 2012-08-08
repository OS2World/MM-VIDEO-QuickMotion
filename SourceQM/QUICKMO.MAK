# quickmo.mak
# Created by IBM WorkFrame/2 MakeMake at 21:39:24 on 25 Jan 1998
#
# The actions included in this make file are:
#  Compile::C++ Compiler
#  Link::Linker
#  Lib::Import Lib

.SUFFIXES: .LIB .cpp .dll .obj 

.all: \
    .\quickmo.LIB

.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /O /Gm /Gd /Ge- /G5 /Gn /C %s

{F:\PRACTICE\QUICKMO\SourceQM}.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /O /Gm /Gd /Ge- /G5 /Gn /C %s

.dll.LIB:
    @echo " Lib::Import Lib "
    implib.exe %|dpfF.LIB %s

{F:\PRACTICE\QUICKMO\SourceQM}.dll.LIB:
    @echo " Lib::Import Lib "
    implib.exe %|dpfF.LIB %s

.\quickmo.dll: \
    .\quickmo.obj \
    {$(LIB)}PROCL.LIB \
    {$(LIB)}MMPM2.LIB \
    {$(LIB)}QUICKMO.DEF
    @echo " Link::Linker "
    icc.exe @<<
     /B" /exepack:2 /packd /optfunc /noe /nod"
     /Fequickmo.dll 
     PROCL.LIB 
     MMPM2.LIB 
     QUICKMO.DEF
     .\quickmo.obj
<<

.\quickmo.obj: \
    F:\PRACTICE\QUICKMO\SourceQM\quickmo.cpp \
    {F:\PRACTICE\QUICKMO\SourceQM;$(INCLUDE);}moovfile.hpp \
    {F:\PRACTICE\QUICKMO\SourceQM;$(INCLUDE);}moovproc.h \
    {F:\PRACTICE\QUICKMO\SourceQM;$(INCLUDE);}moviefil.hpp \
    {F:\PRACTICE\QUICKMO\SourceQM;$(INCLUDE);}moov.hpp \
    {F:\PRACTICE\QUICKMO\SourceQM;$(INCLUDE);}moviefmt.hpp

.\quickmo.LIB: \
    .\quickmo.dll
