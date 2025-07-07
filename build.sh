#!/bin/bash

make clean
make -j4 ADDRESS=0x82000000
make objclean
make -j4 ADDRESS=0x81000000
make -j4 fs.img
cp fs.img fs2.img
