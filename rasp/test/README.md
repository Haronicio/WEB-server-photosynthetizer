# Test de la page WEB seul

Le but de ce test est de :

* crééer une page web test-cursor implémentant un form get avec des curseurs
* crééer une page web test-chart qui créé un graphique se mettant à jour en temp réél
* crééer un header jsonparser qui génère un document json  

## CORS

Cors est un politique WEB qui interdit d'éxécuter certaine fonctions JavaScript or d'une requête HTTP, la fonction qui nous interesse ici est la fonction `fetch` qui récupère un fichier *.json* pour le transformer en objets

Pour notre projet ce n'est pas grave, mais pour le tester certaine partie de l'architecture de notre projet c'est chiant.

## Node js

Node js est une bibliothèque de fonction Javascript, qui créé un server dans lequel s'éxécutera le script JS

`sudo apt install nodejs`

ensuite on créé un fichier *server.js* :

```javascript
var http = require('http');
var fs = require('fs');

const PORT=8080; 

fs.readFile('./test-chart.html', function (err, html) {

    if (err) throw err;    

    http.createServer(function(request, response) {  
        response.writeHeader(200, {"Content-Type": "text/html"});  
        response.write(html);  
        response.end();  
    }).listen(PORT);
});
```

on l'éxécute avec `node server.js`, puis on se connecte avec notre Browser à **http://localhost:8080**
Cela fonctionne de la même manière que CGI finalement.

## NPM

Le problème de node c'est que le seul site parsé sera celui lui par le fichier (j'ai reussi qu'a faire ca), pour cela il faut **npm** qui créé un server HTTP dans le quel on aura tout nos fichier accessible et donc nos dépendances

```console
sudo apt-install npm
npm install http-server -g
```

Pour démarrer le server : `http-server ./`

## jsonparser

Pour mettre à jour la base de donnée, je vais utiliser une bibliothèque C pour JSON appelé **cJSON**, qui est un parser hyper simple à utiliser


Finalement pour éxécuter ce test faire :

`http-server ./` démarre le server sur le port 8080
`./jsonparser/jsonparser` pour démarrer la génération de valeurs aléatoire