# GL3XRender

## What it is?
It is render plugin for DGLE (https://github.com/DGLE-HQ/DGLE). DGLE alredy has it own support for OpenGL but it restricted by version 2.1. 
A plugin allows initilize DGLE under latest OpenGL 4.5 context.

## Installing
Clone and build DGLE.
Navigate to directory where DGLE root is located.
Type
```
git clone https://github.com/k-payl/GL3XRender
```
Folder structures now are:
...
DGLE/
GL3XRender/
...
Now build open Visual Studio solution GL3XRender.sln and build GL3XRender project.
It build GL3XRender.dll to DGLE/bin/windows/plugins/ directory. Now you can use any example from DGLE repository or 
some of my test.

## Project structure
* GL3XRender/src - main codebase
* GL3XRender/_utils -- now it onle constains shader generator
* GL3XRender//_test -- examples for testing functionality 

<p align="left"><img src="https://raw.github.com/k-payl/GL3XRender/teapot.jpg"></p>