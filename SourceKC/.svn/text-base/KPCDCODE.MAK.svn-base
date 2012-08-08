# kpcdcode.mak
# Created by IBM WorkFrame/2 MakeMake at 16:49:53 on 9 Feb 1998
#
# The actions included in this make file are:
#  Compile::C++ Compiler
#  Link::Linker
#  Bind::Resource Bind
#  Compile::Resource Compiler

.SUFFIXES: .cpp .obj .rc .res 

.all: \
    .\kpcdcode.dll

.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /O /Gm /Gd /Ge- /G5 /Gn /C %s

{F:\PRACTICE\QUICKMO\SourceKC}.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /O /Gm /Gd /Ge- /G5 /Gn /C %s

.rc.res:
    @echo " Compile::Resource Compiler "
    rc.exe -r %s .\%|fF.RES

{F:\PRACTICE\QUICKMO\SourceKC}.rc.res:
    @echo " Compile::Resource Compiler "
    rc.exe -r %s .\%|fF.RES

.\kpcdcode.dll: \
    .\kpcdclas.obj \
    .\kpcdcode.obj \
    .\kpcdcode.res \
    {$(LIB)}OS2386.LIB \
    {$(LIB)}PROCL.LIB \
    {$(LIB)}IMAGECOD.LIB \
    {$(LIB)}MMPM2.LIB \
    {$(LIB)}KPCDCODE.DEF
    @echo " Link::Linker "
    @echo " Bind::Resource Bind "
    icc.exe @<<
     /B" /exepack:2 /packd /optfunc /noe /nod"
     /Fekpcdcode.dll 
     OS2386.LIB 
     PROCL.LIB 
     IMAGECOD.LIB 
     MMPM2.LIB 
     KPCDCODE.DEF
     .\kpcdclas.obj
     .\kpcdcode.obj
<<
    rc.exe .\kpcdcode.res kpcdcode.dll

.\kpcdclas.obj: \
    F:\PRACTICE\QUICKMO\SourceKC\kpcdclas.cpp \
    {F:\PRACTICE\QUICKMO\SourceKC;$(INCLUDE);}kpcdcode.h \
    {F:\PRACTICE\QUICKMO\SourceKC;$(INCLUDE);}kpcdclas.hpp \
    {F:\PRACTICE\QUICKMO\SourceKC;$(INCLUDE);}imagecod.hpp \
    {F:\PRACTICE\QUICKMO\SourceKC;$(INCLUDE);}imagecod.h

.\kpcdcode.obj: \
    F:\PRACTICE\QUICKMO\SourceKC\kpcdcode.cpp \
    {F:\PRACTICE\QUICKMO\SourceKC;$(INCLUDE);}kpcdcode.h \
    {F:\PRACTICE\QUICKMO\SourceKC;$(INCLUDE);}kpcdclas.hpp \
    {F:\PRACTICE\QUICKMO\SourceKC;$(INCLUDE);}imagecod.hpp \
    {F:\PRACTICE\QUICKMO\SourceKC;$(INCLUDE);}imagecod.h

.\kpcdcode.res: \
    F:\PRACTICE\QUICKMO\SourceKC\kpcdcode.rc \
    {F:\PRACTICE\QUICKMO\SourceKC;$(INCLUDE)}imagecod.h
