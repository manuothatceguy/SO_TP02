#!/bin/bash
NOMBRE="SO_Docker"


MM_FLAG=""
case $1 in
    "-buddy")
        MM_FLAG="BUDDY"
        ;;
    "-bitmap")
        MM_FLAG="BITMAP"
        ;;
    *)
        MM_FLAG="BUDDY"
        ;;
esac

docker start $NOMBRE
docker exec -it $NOMBRE bash -c "make clean -C /root/Toolchain"
docker exec -it $NOMBRE bash -c "make clean -C /root/"
docker exec -it $NOMBRE bash -c "make MM=$MM_FLAG -C  /root/Toolchain"
docker exec -it $NOMBRE bash -c "make MM=$MM_FLAG -C /root/"
docker stop $NOMBRE