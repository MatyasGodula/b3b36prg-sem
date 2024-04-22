#!/bin/sh

pipe=/tmp/prg-lec09.pipe

rm -rf $pipe
mkfifo $pipe
