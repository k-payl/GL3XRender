# GL3XRender

## What is it?
It is render plugin for [DGLE](http://dglengine.org)https://github.com/DGLE-HQ/DGLE). DGLE alredy has it own support for OpenGL but it restricted by version 2.1. 
A plugin allows initialize DGLE under latest OpenGL 4.5 context.

## Installing
Clone and build DGLE.
Navigate to directory where DGLE root is located.
Type
```
git clone https://github.com/k-payl/GL3XRender
```
Folder structures now are:
```
.../DGLE/
.../GL3XRender/
```
Now build open Visual Studio solution GL3XRender.sln and build GL3XRender project.
It build __GL3XRender.dll__ to __DGLE/bin/windows/plugins/__ directory. Now you can use any example from DGLE repository or 
some of my test.

## Project structure
* GL3XRender/__src__ - main codebase
* GL3XRender/___utils__ - now it only constains shader generator. 
* GL3XRender/___test__ - examples for testing functionality 

![Alt text](teapot.jpg?raw=true "Example Teapot")
