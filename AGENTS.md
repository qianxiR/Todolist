# 项目构建

本项目使用 C++17、Qt Widgets 与 qmake 构建，目标平台为 Windows x64。

- qmake：`D:\anaconda3\Library\bin\qmake.exe`
- C++ 编译器和链接器：Visual Studio Build Tools 的 x64 MSVC
- 构建工具：`nmake`
- MSVC 环境脚本：`C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat`

在项目根目录执行 Release 构建：

```powershell
& 'D:\anaconda3\Library\bin\qmake.exe' 'TodoLIST.pro' -spec win32-msvc 'CONFIG+=release'; $vcvars = 'C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat'; $command = '"' + $vcvars + '" >nul && nmake /nologo /f Makefile.Release'; & cmd.exe /d /s /c $command
```

Release 可执行文件为 `release\Todolist.exe`。构建前须关闭正在运行的该程序，否则链接器无法覆盖文件。
