#!/bin/sh

DIR=$(dirname $0)
export LD_LIBRARY_PATH=$DIR/lib
$DIR/lib/ld-linux.so $DIR/libexec/ebftpd "$@"
