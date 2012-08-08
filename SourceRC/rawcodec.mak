# rawcodec.mak
# Created by IBM WorkFrame/2 MakeMake at 16:56:33 on 9 Feb 1998
#
# The actions included in this make file are:
#  Compile::C++ Compiler
#  Link::Linker
#  Bind::Resource Bind
#  Compile::Resource Compiler

.SUFFIXES: .cpp .obj .rc .res 

.all: \
    .\rawcodec.dll

.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /O /Gm /Gd /Ge- /G5 /Gn /C %s

{F:\PRACTICE\QUICKMO\SourceRC}.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /O /Gm /Gd /Ge- /G5 /Gn /C %s

.rc.res:
    @echo " Compile::Resource Compiler "
    rc.exe -r %s .\%|fF.RES

{F:\PRACTICE\QUICKMO\SourceRC}.rc.res:
    @echo " Compile::Resource Compiler "
    rc.exe -r %s .\%|fF.RES

.\rawcodec.dll: \
    .\rawclass.obj \
    .\rawcodec.obj \
    .\rawcodec.res \
    {$(LIB)}OS2386.LIB \
    {$(LIB)}PROCL.LIB \
    {$(LIB)}IMAGECOD.LIB \
    {$(LIB)}MMPM2.LIB \
    {$(LIB)}RAWCODEC.DEF
    @echo " Link::Linker "
    @echo " Bind::Resource Bind "
    icc.exe @<<
     /B" /exepack:2 /packd /optfunc /noe /nod"
     /Ferawcodec.dll 
     OS2386.LIB 
     PROCL.LIB 
     IMAGECOD.LIB 
     MMPM2.LIB 
     RAWCODEC.DEF
     .\rawclass.obj
     .\rawcodec.obj
<<
    rc.exe .\rawcodec.res rawcodec.dll

.\rawclass.obj: \
    F:\PRACTICE\QUICKMO\SourceRC\rawclass.cpp \
    {F:\PRACTICE\QUICKMO\SourceRC;$(INCLUDE);}rawcodec.h \
    {F:\PRACTICE\QUICKMO\SourceRC;$(INCLUDE);}rawclass.hpp \
    {F:\PRACTICE\QUICKMO\SourceRC;$(INCLUDE);}imagecod.hpp \
    {F:\PRACTICE\QUICKMO\SourceRC;$(INCLUDE);}imagecod.h

.\rawcodec.obj: \
    F:\PRACTICE\QUICKMO\SourceRC\rawcodec.cpp \
    {F:\PRACTICE\QUICKMO\SourceRC;$(INCLUDE);}rawcodec.h \
    {F:\PRACTICE\QUICKMO\SourceRC;$(INCLUDE);}rawclass.hpp \
    {F:\PRACTICE\QUICKMO\SourceRC;$(INCLUDE);}imagecod.hpp \
    {F:\PRACTICE\QUICKMO\SourceRC;$(INCLUDE);}imagecod.h

.\rawcodec.res: \
    F:\PRACTICE\QUICKMO\SourceRC\rawcodec.rc \
    {F:\PRACTICE\QUICKMO\SourceRC;$(INCLUDE)}imagecod.h
