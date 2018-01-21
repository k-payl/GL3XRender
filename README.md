# GL3XRender
![Alt text](teapot.jpg?raw=true "Example Teapot")

## What is it?
It is render plugin for [DGLE](https://github.com/DGLE-HQ/DGLE). DGLE alredy has it own support for OpenGL but it restricted by version 2.1. 
A plugin allows initialize DGLE under latest OpenGL 4.5 context.

## Installing
Clone and build DGLE.
Navigate to directory where DGLE root is located.
Type
```
git clone https://github.com/k-payl/GL3XRender
```
Folder structure now are:
```
.../DGLE/
.../GL3XRender/
```
Now open Visual Studio solution __GL3XRender.sln__ and build GL3XRender project.
It build __GL3XRender.dll__ to DGLE/bin/windows/plugins/ directory. Now you can use any example from DGLE repository or 
some of my test.

## Project structure
* __src__ - main codebase.
* ___utils__ - now it only constains shader generator. 
* ___test__ - examples for testing functionality. 


