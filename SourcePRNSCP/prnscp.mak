# prnscp.mak
# Created by IBM WorkFrame/2 MakeMake at 15:55:02 on 20 Feb 1998
#
# The actions included in this make file are:
#  Compile::C++ Compiler
#  Link::Linker
#  Bind::Resource Bind
#  Compile::Resource Compiler

.SUFFIXES: .cpp .obj .rc .res 

.all: \
    .\prnscp.dll

.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /O /Gm /Gd /Ge- /G5 /C %s

{F:\PRACTICE\QuickFlick\SourcePRNSCP}.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /O /Gm /Gd /Ge- /G5 /C %s

.rc.res:
    @echo " Compile::Resource Compiler "
    rc.exe -r %s .\%|fF.RES

{F:\PRACTICE\QuickFlick\SourcePRNSCP}.rc.res:
    @echo " Compile::Resource Compiler "
    rc.exe -r %s .\%|fF.RES

.\prnscp.dll: \
    .\storefil.obj \
    .\storefmt.obj \
    .\navifile.obj \
    .\navifmt.obj \
    .\prnscp.obj \
    .\prnscp.res \
    {$(LIB)}PROCL.LIB \
    {$(LIB)}OS2386.LIB \
    {$(LIB)}MMPM2.LIB \
    {$(LIB)}PRNSCP.DEF
    @echo " Link::Linker "
    @echo " Bind::Resource Bind "
    icc.exe @<<
     /B" /exepack:2 /optfunc /noe /nod"
     /Feprnscp.dll 
     PROCL.LIB 
     OS2386.LIB 
     MMPM2.LIB 
     PRNSCP.DEF
     .\storefil.obj
     .\storefmt.obj
     .\navifile.obj
     .\navifmt.obj
     .\prnscp.obj
<<
    rc.exe .\prnscp.res prnscp.dll

.\storefil.obj: \
    F:\PRACTICE\QuickFlick\SourcePRNSCP\storefil.cpp \
    {F:\PRACTICE\QuickFlick\SourcePRNSCP;$(INCLUDE);}prnscp.h \
    {F:\PRACTICE\QuickFlick\SourcePRNSCP;$(INCLUDE);}storefil.hpp

.\prnscp.obj: \
    F:\PRACTICE\QuickFlick\SourcePRNSCP\prnscp.cpp \
    {F:\PRACTICE\QuickFlick\SourcePRNSCP;$(INCLUDE);}storefmt.hpp \
    {F:\PRACTICE\QuickFlick\SourcePRNSCP;$(INCLUDE);}prnscp.h \
    {F:\PRACTICE\QuickFlick\SourcePRNSCP;$(INCLUDE);}navifile.hpp \
    {F:\PRACTICE\QuickFlick\SourcePRNSCP;$(INCLUDE);}navifmt.hpp \
    {F:\PRACTICE\QuickFlick\SourcePRNSCP;$(INCLUDE);}storefil.hpp

.\navifmt.obj: \
    F:\PRACTICE\QuickFlick\SourcePRNSCP\navifmt.cpp \
    {F:\PRACTICE\QuickFlick\SourcePRNSCP;$(INCLUDE);}storefmt.hpp \
    {F:\PRACTICE\QuickFlick\SourcePRNSCP;$(INCLUDE);}prnscp.h \
    {F:\PRACTICE\QuickFlick\SourcePRNSCP;$(INCLUDE);}navifmt.hpp

.\navifile.obj: \
    F:\PRACTICE\QuickFlick\SourcePRNSCP\navifile.cpp \
    {F:\PRACTICE\QuickFlick\SourcePRNSCP;$(INCLUDE);}storefmt.hpp \
    {F:\PRACTICE\QuickFlick\SourcePRNSCP;$(INCLUDE);}prnscp.h \
    {F:\PRACTICE\QuickFlick\SourcePRNSCP;$(INCLUDE);}navifile.hpp \
    {F:\PRACTICE\QuickFlick\SourcePRNSCP;$(INCLUDE);}navifmt.hpp \
    {F:\PRACTICE\QuickFlick\SourcePRNSCP;$(INCLUDE);}storefil.hpp

.\storefmt.obj: \
    F:\PRACTICE\QuickFlick\SourcePRNSCP\storefmt.cpp \
    {F:\PRACTICE\QuickFlick\SourcePRNSCP;$(INCLUDE);}storefmt.hpp \
    {F:\PRACTICE\QuickFlick\SourcePRNSCP;$(INCLUDE);}prnscp.h

.\prnscp.res: \
    F:\PRACTICE\QuickFlick\SourcePRNSCP\prnscp.rc \
    {F:\PRACTICE\QuickFlick\SourcePRNSCP;$(INCLUDE)}prnscp.h
