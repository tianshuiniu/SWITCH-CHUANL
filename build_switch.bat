@echo off
REM ============================================================
REM  一键编译 Switch 用的 xcloud_switch.nro
REM  使用方法（给会电脑的大人看）：
REM   1) 先装 devkitPro：百度搜 "devkitPro"，下载 Windows 安装器，
REM      安装时只勾选 "Switch Development"，一路点下一步。
REM   2) 装好后，电脑开始菜单里会多一个 "DevKitPro MSYS"，打开它。
REM   3) 在这个黑窗口里，用  cd  进入本文件所在文件夹，
REM      然后输入：  make
REM   4) 等一会儿，会生成一个  xcloud_switch.nro  文件。
REM  （这个 .bat 是给大人偷懒用的：在 DevKitPro MSYS 里双击它也能编。）
REM ============================================================
cd /d %~dp0
make
pause
