# IMAGCOD.MAK
# Created by IBM WorkFrame/2 MakeMake at 8:53:33 on 24 April 1996
#
# The actions included in this make file are:
#  Compile::C++ Compiler
#  Lib::Library Tool

.SUFFIXES: .cpp .obj 

.all: \
    .\IMAGECOD.LIB

.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /DLINT_ARGS /Tdp /Q /O /Gm /Gd /Ge- /G5 /Gn /Gf /Gi /Gx /Ft- /C %s

.\IMAGECOD.LIB: \
    .\imagecod.obj
    @echo " Lib::Library Tool "
    ilib.exe  /nol /noe IMAGECOD.LIB -+.\imagecod.obj;

.\imagecod.obj: \
    D:\PRACTICE\QUICKMO\SourceIC\imagecod.cpp \
    {D:\PRACTICE\QUICKMO\SourceIC;$(INCLUDE);}imagecod.hpp \
    {D:\PRACTICE\QUICKMO\SourceIC;$(INCLUDE);}imagecod.h
