/*
 *  This file was generated by the SOM Compiler and Emitter Framework.
 *  Generated using template emitter:
 *      SOM Emitter emitctm: 2.23.1.9
 */

#ifndef SOM_Module_practicemoov_Source
#define SOM_Module_practicemoov_Source
#endif
#define PracticeMooV_Class_Source
#define M_PracticeMooV_Class_Source

#include "PracticeMooV.ih"


SOM_Scope ULONG  SOMLINK PracticeMooVX_wpQueryDefaultView(PracticeMooV *somSelf)
{
    ULONG ancestorView;

    PracticeMooVMethodDebug("PracticeMooV","PracticeMooVX_wpQueryDefaultView");

    ancestorView = MMDataFile_parent_WPDataFile_wpQueryDefaultView(somSelf);

    if (ancestorView != OPEN_SETTINGS)
      return (ancestorView);

    return (PracticeMooV_parent_MMMOV_wpQueryDefaultView(somSelf));
}

SOM_Scope HPOINTER  SOMLINK PracticeMooVX_wpQueryAssociatedFileIcon(PracticeMooV *somSelf)
{
    PracticeMooVMethodDebug("PracticeMooV","PracticeMooVX_wpQueryAssociatedFileIcon");

    return (NULLHANDLE);
}


SOM_Scope ULONG  SOMLINK PracticeMooVX_wpclsQueryDefaultView(M_PracticeMooV *somSelf)
{
    ULONG ancestorView;

    M_PracticeMooVMethodDebug("M_PracticeMooV","PracticeMooVX_wpclsQueryDefaultView");

    ancestorView = M_MMDataFile_parent_M_WPDataFile_wpclsQueryDefaultView(somSelf);

    if (ancestorView != OPEN_SETTINGS)
      return (ancestorView);

    return (M_PracticeMooV_parent_M_MMMOV_wpclsQueryDefaultView(somSelf));
}

SOM_Scope PSZ  SOMLINK PracticeMooVX_wpclsQueryTitle(M_PracticeMooV *somSelf)
{
    static char title[256] = "QuickTime Movie";

    M_PracticeMooVMethodDebug("M_PracticeMooV","PracticeMooVX_wpclsQueryTitle");

    return (title);
}
