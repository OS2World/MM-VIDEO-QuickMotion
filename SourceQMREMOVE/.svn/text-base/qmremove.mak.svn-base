# qmremove.mak
# Created by IBM WorkFrame/2 MakeMake at 19:16:01 on 28 April 1997
#
# The actions included in this make file are:
#  Compile::C++ Compiler
#  Link::Linker

.SUFFIXES: .cpp .obj 

.all: \
    .\qmremove.dll

.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /Rn /Ge- /G5 /C %s

{F:\PRACTICE\quickmo\sourceqmremove}.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /Rn /Ge- /G5 /C %s

.\qmremove.dll: \
    .\qmremove.obj \
    {$(LIB)}MMPM2.LIB \
    {$(LIB)}QMREMOVE.DEF
    @echo " Link::Linker "
    icc.exe @<<
     /B" /exepack:2 /packd /optfunc /noe"
     /Feqmremove.dll 
     MMPM2.LIB 
     QMREMOVE.DEF
     .\qmremove.obj
<<

.\qmremove.obj: \
    F:\PRACTICE\quickmo\sourceqmremove\qmremove.cpp
