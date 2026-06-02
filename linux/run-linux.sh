#!/bin/bash

IMG_NAME="ubuntu.26.04.base"
SCRIPT_DIR=$(dirname "$0")

docker build -t $IMG_NAME $SCRIPT_DIR
echo "docker image built"


docker run -it --rm -v $SCRIPT_DIR/..:/workspace $IMG_NAME