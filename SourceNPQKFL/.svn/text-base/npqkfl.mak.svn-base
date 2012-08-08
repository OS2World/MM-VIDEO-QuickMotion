# npqkfl.mak
# Created by IBM WorkFrame/2 MakeMake at 23:13:47 on 20 Feb 1998
#
# The actions included in this make file are:
#  Compile::C++ Compiler
#  Link::Linker
#  Bind::Resource Bind
#  Compile::Resource Compiler

.SUFFIXES: .cpp .obj .rc .res 

.all: \
    .\npqkfl.dll

.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /O /Gm /Gd /Ge- /G5 /C %s

{F:\PRACTICE\QuickFlick\SourceNPQKFL}.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /O /Gm /Gd /Ge- /G5 /C %s

.rc.res:
    @echo " Compile::Resource Compiler "
    rc.exe -r %s .\%|fF.RES

{F:\PRACTICE\QuickFlick\SourceNPQKFL}.rc.res:
    @echo " Compile::Resource Compiler "
    rc.exe -r %s .\%|fF.RES

.\npqkfl.dll: \
    F:\PRACTICE\QuickFlick\SourceNPQKFL\npos2.obj \
    .\qfplugin.obj \
    .\about.obj \
    .\npqkfl.obj \
    .\npqkfl.res \
    {$(LIB)}PROCL.LIB \
    {$(LIB)}OS2386.LIB \
    {$(LIB)}MMPM2.LIB \
    {$(LIB)}QUIKFLIK.LIB \
    {$(LIB)}NPQKFL.DEF
    @echo " Link::Linker "
    @echo " Bind::Resource Bind "
    icc.exe @<<
    /Tdp 
     /O /Gm /Gd /Ge- /G5 
     /B" /exepack:2 /packd /optfunc /noe /nod"
     /Fenpqkfl.dll 
     PROCL.LIB 
     OS2386.LIB 
     MMPM2.LIB 
     QUIKFLIK.LIB 
     NPQKFL.DEF
     F:\PRACTICE\QuickFlick\SourceNPQKFL\npos2.obj
     .\qfplugin.obj
     .\about.obj
     .\npqkfl.obj
<<
    rc.exe .\npqkfl.res npqkfl.dll

.\npqkfl.obj: \
    F:\PRACTICE\QuickFlick\SourceNPQKFL\npqkfl.cpp \
    {F:\PRACTICE\QuickFlick\SourceNPQKFL;$(INCLUDE);}npqkfl.h \
    {F:\PRACTICE\QuickFlick\SourceNPQKFL;$(INCLUDE);}qfplugin.hpp \
    {F:\PRACTICE\QuickFlick\SourceNPQKFL;$(INCLUDE);}quikflik.hpp \
    {F:\PRACTICE\QuickFlick\SourceNPQKFL;$(INCLUDE);}pluginio.hpp \
    {F:\PRACTICE\QuickFlick\SourceNPQKFL;$(INCLUDE);}about.hpp \
    {F:\PRACTICE\QuickFlick\SourceNPQKFL;$(INCLUDE);}quikflik.h

.\about.obj: \
    F:\PRACTICE\QuickFlick\SourceNPQKFL\about.cpp \
    {F:\PRACTICE\QuickFlick\SourceNPQKFL;$(INCLUDE);}npqkfl.h \
    {F:\PRACTICE\QuickFlick\SourceNPQKFL;$(INCLUDE);}quikflik.hpp \
    {F:\PRACTICE\QuickFlick\SourceNPQKFL;$(INCLUDE);}about.hpp \
    {F:\PRACTICE\QuickFlick\SourceNPQKFL;$(INCLUDE);}quikflik.h

.\qfplugin.obj: \
    F:\PRACTICE\QuickFlick\SourceNPQKFL\qfplugin.cpp \
    {F:\PRACTICE\QuickFlick\SourceNPQKFL;$(INCLUDE);}npqkfl.h \
    {F:\PRACTICE\QuickFlick\SourceNPQKFL;$(INCLUDE);}qfplugin.hpp \
    {F:\PRACTICE\QuickFlick\SourceNPQKFL;$(INCLUDE);}qfcontrol.hpp \
    {F:\PRACTICE\QuickFlick\SourceNPQKFL;$(INCLUDE);}qfplayer.hpp \
    {F:\PRACTICE\QuickFlick\SourceNPQKFL;$(INCLUDE);}quikflik.hpp \
    {F:\PRACTICE\QuickFlick\SourceNPQKFL;$(INCLUDE);}pluginio.hpp \
    {F:\PRACTICE\QuickFlick\SourceNPQKFL;$(INCLUDE);}about.hpp \
    {F:\PRACTICE\QuickFlick\SourceNPQKFL;$(INCLUDE);}quikflik.h

.\npqkfl.res: \
    F:\PRACTICE\QuickFlick\SourceNPQKFL\npqkfl.rc \
    {F:\PRACTICE\QuickFlick\SourceNPQKFL;$(INCLUDE)}NPQKFL.ICO \
    {F:\PRACTICE\QuickFlick\SourceNPQKFL;$(INCLUDE)}npqkfl.h
