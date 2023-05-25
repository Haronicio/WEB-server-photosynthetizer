
function matrixToKeyValue(matrix) {
    let result = {};
    for (var i = 0; i < matrix.length; i++) {
        var key = matrix[i][0];
        var value = parseInt(matrix[i][1]) || 0; //si Nan alors c'est 0
        result[key] = value;
    }
    return result;
}

function getURIelements() {
    //Récupère la chaîne de requête de l'URL, supprime le "?" de la chaîne de requête
    //Sépare en un tableau de chaines (&) et divise la chaîne de requête en un tableau de paires de clé-valeur (=)
    //ensuite transformé en un ensemble
    return matrixToKeyValue(window.location.search.substring(1).split("&").map(function(str){return str.split("=");}));
}


//fetch la base de donnée et retourne un tableau lisible par le chart
function fetchBD(jsonFile) {
    
    let res = [];
    fetch(jsonFile)
    .then(response => response.json()) // Convertit la réponse en un objet JSON
    .then(data => {
        // Une fois le fichier JSON lu avec succès, vous pouvez accéder à ses données.
        // Utilisez les propriétés et les valeurs de l'objet "data" selon votre structure JSON.

        const titreAbscisse = data.abscisse.titre;
        const titreOrdonnee = data.ordonnee.titre;
        const coordonnees = data.coordonnees;

        // console.log("Titre pour l'abscisse :", titreAbscisse);
        // console.log("Titre pour l'ordonnée :", titreOrdonnee);
        // console.log("Coordonnées :", coordonnees);
        // console.log(res);
        res.push([titreAbscisse,titreOrdonnee]);
        (coordonnees.map(function (params) {
            return [params['x'],params['y']];
         })).forEach(element => {
            res.push(element);            
         });
    })
    .catch(error => {
        // En cas d'erreur lors de la récupération ou de la lecture du fichier JSON.
        console.error('Une erreur s\'est produite:', error);
    });

    return res;     
}

//transforme une chaine JSON en tableau formaté pour être affiché
function formatToChart(data)
{
    let res = [];
    let titreAbscisse = data.abscisse.titre;
    let titreOrdonnee = data.ordonnee.titre;
    let coordonnees = data.coordonnees;

    res.push([titreAbscisse,titreOrdonnee]);
    (coordonnees.map(function (params) {
        return [params['x'],params['y']];
        })).forEach(element => {
        res.push(element);            
        });
    
    return res;

}

//fonction qui surveille un fichier toutes les intervall, s'il est modifié, on apelle un callback
function fetchJSON_whenUpdate(filePath, interval, callback) {
  let lastModifiedTime = '';

  function fetchData() {
    fetch(filePath, {
      headers: {
        'If-Modified-Since': lastModifiedTime
      }
    })
    .then(response => {
      if (response.status === 200) {
        // File has been modified
        lastModifiedTime = response.headers.get('Last-Modified');
        return response.json();
      } else if (response.status === 304) {
        // File has not been modified
        return null;
      } else {
        throw new Error('Error reading JSON file.');
      }
    })
    .then(data => {
      if (data) {
        // Process the updated data
        callback(data);
        // Clear data
        data = {};
      }
      // Fetch data again after the specified interval
      setTimeout(fetchData, interval);
    })
    .catch(error => {
      console.error('An error occurred:', error);
    });
  }

  // Initial fetch
  fetchData();
}
