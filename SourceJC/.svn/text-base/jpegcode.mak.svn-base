# jpegcode.mak
# Created by IBM WorkFrame/2 MakeMake at 16:49:02 on 9 Feb 1998
#
# The actions included in this make file are:
#  Compile::C++ Compiler
#  Link::Linker
#  Bind::Resource Bind
#  Compile::Resource Compiler

.SUFFIXES: .cpp .obj .rc .res 

.all: \
    .\jpegcode.dll

.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /O /Gm /Gd /Ge- /G5 /Gn /C %s

{F:\PRACTICE\QUICKMO\SourceJC}.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /O /Gm /Gd /Ge- /G5 /Gn /C %s

.rc.res:
    @echo " Compile::Resource Compiler "
    rc.exe -r %s .\%|fF.RES

{F:\PRACTICE\QUICKMO\SourceJC}.rc.res:
    @echo " Compile::Resource Compiler "
    rc.exe -r %s .\%|fF.RES

.\jpegcode.dll: \
    .\jfifcode.obj \
    .\jpegcode.obj \
    .\bufmgr.obj \
    .\jpegcode.res \
    {$(LIB)}OS2386.LIB \
    {$(LIB)}PROCL.LIB \
    {$(LIB)}IMAGECOD.LIB \
    {$(LIB)}LIBJPEG.LIB \
    {$(LIB)}MMPM2.LIB \
    {$(LIB)}JPEGCODE.DEF
    @echo " Link::Linker "
    @echo " Bind::Resource Bind "
    icc.exe @<<
     /B" /exepack:2 /packd /optfunc /noe /nod"
     /Fejpegcode.dll 
     OS2386.LIB 
     PROCL.LIB 
     IMAGECOD.LIB 
     LIBJPEG.LIB 
     MMPM2.LIB 
     JPEGCODE.DEF
     .\jfifcode.obj
     .\jpegcode.obj
     .\bufmgr.obj
<<
    rc.exe .\jpegcode.res jpegcode.dll

.\jfifcode.obj: \
    F:\PRACTICE\QUICKMO\SourceJC\jfifcode.cpp \
    {F:\PRACTICE\QUICKMO\SourceJC;$(INCLUDE);}jpegcode.h \
    {F:\PRACTICE\QUICKMO\SourceJC;$(INCLUDE);}jfifcode.hpp \
    {F:\PRACTICE\QUICKMO\SourceJC;$(INCLUDE);}imagecod.hpp \
    {F:\PRACTICE\QUICKMO\SourceJC;$(INCLUDE);}imagecod.h

.\bufmgr.obj: \
    F:\PRACTICE\QUICKMO\SourceJC\bufmgr.cpp \
    {F:\PRACTICE\QUICKMO\SourceJC;$(INCLUDE);}jpegcode.h

.\jpegcode.obj: \
    F:\PRACTICE\QUICKMO\SourceJC\jpegcode.cpp \
    {F:\PRACTICE\QUICKMO\SourceJC;$(INCLUDE);}jpegcode.h \
    {F:\PRACTICE\QUICKMO\SourceJC;$(INCLUDE);}jfifcode.hpp \
    {F:\PRACTICE\QUICKMO\SourceJC;$(INCLUDE);}imagecod.hpp \
    {F:\PRACTICE\QUICKMO\SourceJC;$(INCLUDE);}imagecod.h

.\jpegcode.res: \
    F:\PRACTICE\QUICKMO\SourceJC\jpegcode.rc \
    {F:\PRACTICE\QUICKMO\SourceJC;$(INCLUDE)}imagecod.h
