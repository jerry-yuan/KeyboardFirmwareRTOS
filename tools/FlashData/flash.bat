@echo off

SET SERIAL_PORT=com7
SET SERIAL_BAUDRATE=115200

echo �����Բ�����%SERIAL_BAUDRATE%ˢдFlash��λ��%SERIAL_PORT%��Flash


.\FlashTool��λ��\flashrom.exe -p serprog:dev=\\.\com7:115200 -w ./output.bin
pause