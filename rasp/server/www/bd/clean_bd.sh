#!/usr/bin/bash


touch "bd_$(date +"%F %T").json"
cp ./bd.json "bd_$(date +"%F %T").json"
cp ./bd_base.json ./bd.json