@echo off

SET FLASH_SIZE=8192
SET SERIAL_PORT=com10
SET SERIAL_BAUDRATE=115200

echo ��������FAT32����...

mkfatimg root output.img %FLASH_SIZE% 512

echo �����Բ�����%SERIAL_BAUDRATE%ˢдFlash��λ��%SERIAL_PORT%��Flash


.\FlashTool��λ��\flashrom.exe -p serprog:dev=\\.\com10:115200 -w ./output.img
pause