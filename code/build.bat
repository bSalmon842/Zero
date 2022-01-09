@echo off

set commonFlagsCompiler=-MD -nologo -Gm- -GR- -fp:fast -EHa- -Od -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -wd4996 -FC -Z7 -DZERO_INTERNAL=1 -DZERO_SLOW=1
set commonFlagsCompilerNoWarn=-MDd -nologo -Gm- -GR- -fp:fast -EHa- -Od -Oi -WX -W4 -FC -Z7 -DZERO_INTERNAL=1 -DZERO_SLOW=1
set commonFlagsLinker= -incremental:no -opt:ref

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build
cl %commonFlagsCompiler% ..\code\zero.cpp ..\code\imgui\imgui*.cpp ..\code\imgui\implot*.cpp ..\code\glad\glad.c /link %commonFlagsLinker% opengl32.lib glfw3.lib gdi32.lib shell32.lib
echo.
cl %commonFlagsCompiler% ..\code\zero_trackmapper.cpp ..\code\re.c /link %commonFlagsLinker%
popd
