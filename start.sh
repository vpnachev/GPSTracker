#!/bin/bash

mkdir -p /data/db
mongod &>>mongod.log &
node collector/TrackerServer.js &>>collector.log &
