@echo off

..\sdcc\bin\sdcc.exe -pstm8l151g4 -mstm8 -V --verbose --std-sdcc99 -DSTM8L151G -I. -I../sdcc/include -I../sdcc/lib/stm8 -o build/usart.rel -c usart.c
..\sdcc\bin\sdcc.exe -pstm8l151g4 -mstm8 -V --verbose --std-sdcc99 -DSTM8L151G -I. -I../sdcc/include -I../sdcc/lib/stm8 -o build/spi.rel -c spi.c
..\sdcc\bin\sdcc.exe -pstm8l151g4 -mstm8 -V --verbose --std-sdcc99 -DSTM8L151G -I. -I../sdcc/include -I../sdcc/lib/stm8 -o build/cc1101.rel -c cc1101.c
..\sdcc\bin\sdcc.exe -pstm8l151g4 -mstm8 -V --verbose --std-sdcc99 -DSTM8L151G -I. -I../sdcc/include -I../sdcc/lib/stm8 -o build/remote.hex main.c build/usart.rel build/spi.rel build/cc1101.rel