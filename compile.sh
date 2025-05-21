#!/bin/bash
NOMBRE="SO_Docker"

# Check if buddy flag is provided
BUDDY_FLAG=""
if [ "$1" = "-buddy" ]; then
    BUDDY_FLAG="BUDDY"
fi

docker start $NOMBRE
docker exec -it $NOMBRE bash -c "make clean -C /root/Toolchain"
docker exec -it $NOMBRE bash -c "make clean -C /root/"
docker exec -it $NOMBRE bash -c "make MM=$BUDDY_FLAG -C  /root/Toolchain"
docker exec -it $NOMBRE bash -c "make MM=$BUDDY_FLAG -C /root/"
docker stop $NOMBRE