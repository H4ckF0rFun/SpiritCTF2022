#!/bin/bash
rm ./pwn
gcc ./main.c -fno-stack-protector -no-pie -o ./pwn

