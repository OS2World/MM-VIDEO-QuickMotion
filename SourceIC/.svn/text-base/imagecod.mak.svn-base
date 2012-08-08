# imagecod.mak
# Created by IBM WorkFrame/2 MakeMake at 23:07:51 on 3 June 1996
#
# The actions included in this make file are:
#  Compile::C++ Compiler
#  Lib::Library Tool

.SUFFIXES: .cpp .obj 

.all: \
    .\imagecod.lib

.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /DLINT_ARGS /Tdp /Q /O /Gm /Gd /Ge- /G5 /Gn /Gf /Gi /Gx /Ft- /C %s

{D:\PRACTICE\QUICKMO\SourceIC}.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /DLINT_ARGS /Tdp /Q /O /Gm /Gd /Ge- /G5 /Gn /Gf /Gi /Gx /Ft- /C %s

.\imagecod.lib: \
    .\imagecod.obj
    @echo " Lib::Library Tool "
    ilib.exe  /nol /noe imagecod.lib -+.\imagecod.obj;

.\imagecod.obj: \
    D:\PRACTICE\QUICKMO\SourceIC\imagecod.cpp \
    {D:\PRACTICE\QUICKMO\SourceIC;$(INCLUDE);}imagecod.hpp \
    {D:\PRACTICE\QUICKMO\SourceIC;$(INCLUDE);}imagecod.h
