# README pour test MQTT gateway

## Introduction

Le principe de ce test est de :

* installer un mosquitto broker pour la distribution
* installer libmosquitto pour la library mosquitto
* créer un client subscribe à esp2pi et qui publish sur pi2esp
* créer un tester client subscribe à pi2esp et qui publish sur esp2pi
* si le pipe du server à une nouvelle info on l'a transmet à l'esp
* si on recoit un msg de l'esp on l'écrit dans la base de donnée
* une page html/JS qui lit bd et mets à jour le graphique sur la page

## Installation moquitto

`sudo apt-get install mosquitto mosquitto-clients`

Cn créé un fichier de configuration *mosquitto.conf*

`mosquitto -c mosquitto.conf`

## Installation lib mosquitto

`sudo apt-get update`
`sudo apt-get install libmosquitto-dev`

## client pi et client esp

On créé 3 handler pour la connexion, la deconnexion et à la reception de message.
à la connexion on abonne notre client avec mosquitto_subscribe
à la deconnexion on reconnecte notre client mosquitto_reconnect

dans le main on initialise mosquitto,on créé un client avec mosquitto_new, on assigne les callbacks aux handlers, on se connecte au brocker et on démarre le thread de mosquitto avec mosquitto_loop_start

dans une boucle infinie on publie un msg

## base de donnée et pipe

On crée un fichier *writer.py* qui écrit dans un fd s2g (données provenant du serveur)

On édite *gatewayMQTT.c* pour qu'il ouvre deux fd, s2g en lecture et bd_w en écriture vers la base de donnée

à la réception d'un msg de esp on écrit dans le fichier pointé par bd_w
lorsqu'une donnée est dans le pipe, on envoi un msg à esp

## Problème connu sur le raspberry pi

tout d'abord sur le raspberry pi :

`sudo apt-get update`
`sudo apt-get install libmosquitto-dev`
`sudo apt-get install mosquitto mosquitto-clients`

le raspberry à un problème avec les permissions et les pipes ... voici une solution :

* supprimer le pipe déffecteux `rm /tmp/s2g`
* lancer gateway en mode admin `sudo gatewayMQTT` dans *gateway/*
* lancer le serveur en admin `sudo python3 ../server.py` dans *server/www/*

