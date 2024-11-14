#!/bin/bash
NOMBRE="tpe_arqui_g17"
docker start $NOMBRE
docker exec -it $NOMBRE bash -c "make clean -C /root/Toolchain"
docker exec -it $NOMBRE bash -c "make clean -C /root/"
docker exec -it $NOMBRE bash -c "make -C /root/Toolchain"
docker exec -it $NOMBRE bash -c "make -C /root/"
#docker stop $NOMBRE