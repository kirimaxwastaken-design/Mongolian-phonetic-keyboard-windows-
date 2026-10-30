#!/bin/sh
# Linux/macOS дээр Windows-д зориулж build хийх: apt install mingw-w64 nsis
set -e
cd "$(dirname "$0")"
mkdir -p build
(cd tools && python3 gen_tables.py && mv tables.h ../src/tables.h)
(cd installer && x86_64-w64-mingw32-windres app.rc -O coff -o ../build/app.o)
x86_64-w64-mingw32-gcc -O2 -Wall -mwindows -s -static -o build/mongolian-phonetic.exe \
  src/main.c src/engine.c build/app.o -lshell32 -luser32
(cd installer && makensis -V1 installer.nsi)
echo "OK: build/MongolianPhonetic-Setup.exe"
