#!/usr/bin/python3

import cgi, os, time,sys
from http import cookies

#nettoyage de la base de donnée si trop grosse (jamais trigger ??!) voire jsonparser.c dans le gateway
if(os.path.getsize("bd/bd.json") > 100000):
  print('\a')
  os.system("cd ../bd && ./clean_bd.sh")

#récupération du form
form = cgi.FieldStorage()
val_min = form.getvalue('valeur_min')
val_max = form.getvalue('valeur_max')
val_curs = form.getvalue('valeur-curseur')
val_gamme = form.getvalue('choix_gamme')
val_cle = form.getvalue('choix_cle')
    

#chargement du pipe fifo
s2gName = '/tmp/s2g'
s2g = open(s2gName,'w')

 #écriture dans les pipe
s2g.write("%s %s %s %s %s\n" % (val_min,val_max,val_curs,val_gamme,val_cle))
s2g.flush()
s2g.close()

    
    #code html
html="""
<!doctype html>
<html>

<head>
    <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
  <script src="https://code.jquery.com/jquery-3.6.0.min.js"></script>
  <script src="https://code.jquery.com/ui/1.13.0/jquery-ui.min.js"></script>
  <script type="text/javascript" src="https://www.google.com/jsapi"></script>
  <link rel="stylesheet" href="https://code.jquery.com/ui/1.13.0/themes/base/jquery-ui.css">

  
  <script src="../src/js/func.js">/*l'ensemble des fonctions tierce*/</script>

  <script type="text/javascript">
    // Le code à été réarrangé car il consommait une énorme quantité de mémoire (> 4Go !!!!)
    //Cependant il y a beaucoup d'ereur de lecture mais le code est optimisé et fonctionne
    //Et la lecture du graphe sera de plus en plus lente 
    
    let resolution = 127;
    
    var params = getURIelements();
    google.load("visualization", "1", { packages: ["corechart"] });
    google.setOnLoadCallback(function () {
      var options = {
        title: 'lumiere',
        hAxis: { title: ' ', titleTextStyle: { color: '#333' } },
        vAxis: { minValue: 0, maxValue: 100 }
      };
      var chart = new google.visualization.AreaChart(document.getElementById('chart'));

      function updateChart(data) {
        var chartData = google.visualization.arrayToDataTable(formatToChart(data));
        chart.draw(chartData, options);
      }

      function fetchDataAndDrawChart() {
        fetchJSON_whenUpdate('../bd/bd.json', 250, updateChart);
      }

      fetchDataAndDrawChart();
      setInterval(fetchDataAndDrawChart, 250);
    });
    //console.log(params);
    function dessinerLignes() {
        var canvas = document.getElementById('canvas');
        var context = canvas.getContext('2d');
        var hauteur = 50;
        // Effacer le contenu précédent du canvas
        context.clearRect(0, 0, canvas.width, canvas.height);
    
        // Dessiner la première ligne
        context.beginPath();
        context.moveTo(0,hauteur);
        context.lineTo(0 + params["valeur_min"], hauteur);
        context.lineTo(params["valeur_min"], 0);
        context.lineTo(params["valeur_max"], 0);
        context.lineTo(params["valeur_max"], hauteur);
        context.lineTo(resolution, hauteur);
        context.strokeStyle = 'black';
        context.lineWidth = 2;
        context.stroke();

      }

    $(function(){
        $( "#choix_gamme" ).selectmenu({ change: function( event, ui ) { $("#myForm").submit(); }});
        $( "#choix_cle" ).selectmenu({ change: function( event, ui ) { $("#myForm").submit(); }});

    });

    $(function () {
      $("#range").slider({
        range: true,
        min: 0,
        max: resolution,
        values: [(params["valeur_min"]) ? params["valeur_min"]:25, (params["valeur_max"]) ? params["valeur_max"]:75],
        // Ajouter une fonction de rappel qui met à jour les valeurs cachées dans le formulaire
        slide: function (event, ui) {
          $("#valeur_min").val(ui.values[0]);
          $("#valeur_max").val(ui.values[1]);
          $("#valeur-curseur").val(params["valeur-curseur"]);
        },
        stop: function (event, ui) {
            $("#myForm").submit(); // Envoyer le formulaire à chaque relachement
          }
      });
    });
    $(function () {
      $("#slider-vertical").slider({
        orientation: "vertical",
        min: 0,
        max: 100,
        value: (params["valeur-curseur"]) ? params["valeur-curseur"]:50,
        slide: function (event, ui) {
          $("#valeur_min").val(params["valeur_min"]);
          $("#valeur_max").val(params["valeur_max"]);
          $("#valeur-curseur").val(ui.value);
        },
        stop: function (event, ui) {
            $("#myForm").submit(); // Envoyer le formulaire à chaque relachement
          }
      });
    });
    setInterval(dessinerLignes,100);

    window.addEventListener("load", (event) => {
        document.getElementById("choix_cle").value = params["choix_cle"];
        document.getElementById("choix_gamme").value = params["choix_gamme"];
      });
    
  </script>
</head>

<body>
  <div id="selector">
    <label for="range">Duty :</label>
    <div id="range"></div>
    <label for="slider-vertical">Mod :</label>
    <div id="slider-vertical"></div>
    <canvas id="canvas"></canvas>
  </div>

  <form id="myForm" method="get" action="main.py">
    <input type="hidden" name="valeur_min" id="valeur_min">
    <input type="hidden" name="valeur_max" id="valeur_max">
    <input type="hidden" id="valeur-curseur" name="valeur-curseur">
    <!-- <input type="hidden" name="choix_gamme" id="choix_gamme" value="">
    <input type="hidden" name="choix_cle" id="choix_cle" value=""> -->
    <select name="choix_gamme" id="choix_gamme" >
      <option value="0">Chromatique</option>
      <option value="1">Majeure</option>
      <option value="2">Mineure</option>
      <option value="3">Pentatonique Majeure</option>
    </select>
    <select name="choix_cle" id="choix_cle">
      <option value="0">C</option>
      <option value="1">C#</option>
      <option value="2">D</option>
      <option value="3">Eb</option>
      <option value="4">E</option>
      <option value="5">F</option>
      <option value="6">F#</option>
      <option value="7">G</option>
      <option value="8">G#</option>
      <option value="9">A</option>
      <option value="10">Bb</option>
      <option value="11">B</option>
    </select>
  </form>
  <label for="chart">PhotoRésitance :</label>
  
  <div id="chart" style="width: 600; height: 300px;"></div>
</body>

</html>
"""

print(html)
