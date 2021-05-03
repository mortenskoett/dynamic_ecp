#! /usr/bin/env bash

echo "Will remove all docker images..."
read -p "Press enter to continue"

docker ps -a -q | xargs docker rm
docker rmi $(docker images -a -q)
