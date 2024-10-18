docker pull agodio/itba-so:2.0
docker run -v ${PWD}:/root --security-opt seccomp:unconfined --name tpe_arqui_g17 -it agodio/itba-so:2.0 
