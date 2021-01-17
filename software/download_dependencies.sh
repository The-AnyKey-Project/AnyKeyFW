#!/bin/bash
. env_vars
mkdir .3rdparty
cd .3rdparty
wget https://osdn.net/dl/chibios/ChibiOS_$OS_VERSION.7z
7z x ChibiOS_$OS_VERSION.7z 
git clone https://github.com/olikraus/u8g2.git 
git -C u8g2 checkout 9ecb87c
cd -
