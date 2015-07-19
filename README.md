# Particle_Rky1
Particle (formely Spark core) : 

Intégration des éléments suivant :
 - ecran TFT couleur 1.8"
 - MPU5065 : accelerometre + gyroscope 6 axes
 - DHT11 : capteur humidité / température
 - Luminosité : capteur lumière
 - click boutons
 - interruption
 
Fonctions mise en oeuvre :
 - menu déroulant
 - séquençage du temps réel par signature
 - trace de courbe (mémorisation EEPROM test sur Particle Photon)
 - affichage valeur 0-100% sur cadran
 - enregistrements de données (toutes les minutes)
 - transfert séries données accélerometre vers PC (interface Processing) visualisation oscilloscope
 - webhooks data intégration avec librato
 - visualisation variables web dans google spreadsheet
 - fonction de commande à partir curl ou intégration IFTT et DoButton
 
Next Step :
 - communication entre deux Spark / Particle

v0 (c) e-Coucou 2015
