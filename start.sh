#!/bin/bash

mkdir -p /data/db
mongod &>>mongod.log &
cd /root/collector
npm install
npm start
