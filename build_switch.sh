#!/bin/bash
# 给用 Linux / macOS 的大人：
#   1) 安装 devkitPro：
#        sudo (dkp-)pacman -S switch-dev switch-curl switch-mbedtls
#   2) 进入本脚本所在文件夹，运行：  bash build_switch.sh
#   产物：xcloud_switch.nro
cd "$(dirname "$0")"
export DEVKITPRO=/opt/devkitpro
make
