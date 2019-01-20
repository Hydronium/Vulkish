#20170317 created
#As of 20170912, this makefile builds the program correctly.

INCLUDE = $(PellesCDir)\Include\Win;$(PellesCDir)\Include
LIB = $(PellesCDir)\Lib\Win64;$(PellesCDir)\Lib

#Looked up LoadIcon on MSDN and saw that it was part of User32.lib, so added it in here
#wgl functions supposedly exist in opengl32...which is loadewd with LoadLibrary..
#LINKFLAGS = gdi32.lib User32.lib kernel32.lib advapi32.lib delayimp64.lib /VERBOSE 
LINKFLAGS = /debug /debugtype:po /subsystem:windows /machine:x64 /verbose User32.lib kernel32.lib advapi32.lib delayimp64.lib

Vulkish.exe : Vulkish.obj 
	polink.exe $(LINKFLAGS) $**

#To avoid issues with "target architecture is not defined", you need /Ze which will define it based on what /T is
#/Ze = enables microsoft extensions to C, required on windows computers
Vulkish.obj : _main.c
	pocc.exe /std:C17 /Tx64-coff /arch:SSE2 /Zi /Ob1 /fp:precise /W2 /Ze $** /Fo Vulkish.obj