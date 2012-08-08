# ima4.mak
# Created by IBM WorkFrame/2 MakeMake at 20:18:45 on 8 April 1997
#
# The actions included in this make file are:
#  Compile::C++ Compiler
#  Lib::Library Tool

.SUFFIXES: .cpp .obj 

.all: \
    .\ima4.lib

.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /O /Gm /Ge- /G5 /Ft- /C %s

{F:\PRACTICE\QUICKMO\SourceIMA4}.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /O /Gm /Ge- /G5 /Ft- /C %s

.\ima4.lib: \
    .\ima4.obj
    @echo " Lib::Library Tool "
    ilib.exe  /nol ima4.lib @ima4.rf ;

.\ima4.obj: \
    F:\PRACTICE\QUICKMO\SourceIMA4\ima4.cpp \
    {F:\PRACTICE\QUICKMO\SourceIMA4;$(INCLUDE);}ima4.h
