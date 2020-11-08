@echo off

SET FLASH_SIZE=8192
SET SERIAL_PORT=com10
SET SERIAL_BAUDRATE=115200

echo 正在生成FAT32镜像...

mkfatimg root output.img %FLASH_SIZE% 512

echo 尝试以波特率%SERIAL_BAUDRATE%刷写Flash至位于%SERIAL_PORT%的Flash


.\FlashTool上位机\flashrom.exe -p serprog:dev=\\.\com10:115200 -w ./output.img
pause