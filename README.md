# Todolist

基于 C++17 和 Qt Widgets 的本地四象限任务管理工具。

## 功能

- 按“立即处理、计划安排、委派处理、暂不处理”管理任务。
- 为每个任务维护可勾选的计划项；完成父任务时同步完成全部计划项。
- 支持新增、编辑、删除与拖动排序任务。
- 任务数据以 JSON 保存在当前用户的应用数据目录。
- 支持最小化到系统托盘与开机启动设置。

## 构建

构建环境：Qt Widgets、qmake、Visual Studio Build Tools x64 MSVC 和 nmake。

```powershell
& 'D:\anaconda3\Library\bin\qmake.exe' 'TodoLIST.pro' -spec win32-msvc 'CONFIG+=release'; $vcvars = 'C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat'; $command = '"' + $vcvars + '" >nul && nmake /nologo /f Makefile.Release'; & cmd.exe /d /s /c $command
```

Release 可执行文件为 `release\Todolist.exe`。构建前须关闭正在运行的程序。
