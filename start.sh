#!/bin/bash
ROOT_PATH=`pwd`

export LD_LIBRARY_PATH=$ROOT_PATH/lib; ./httpd 127.0.0.1 8080 
