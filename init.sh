docker pull agodio/itba-so-multi-platform:3.0
docker run -v ${PWD}:/root --security-opt seccomp:unconfined --name SO_Docker -ti agodio/itba-so-multi-platform:3.0 
