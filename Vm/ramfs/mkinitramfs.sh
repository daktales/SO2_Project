#!/bin/sh

find . | cpio -o -H newc | gzip > $1
