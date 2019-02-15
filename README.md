# Teleinfo Universal Library

Adaptation de la fameuse librairie Teleinfo (https://github.com/hallard/LibTeleinfo) pour prendre en compte le mode Standard en plus du mode Historique (Linky est arrivé à la maison et je souhaite utiliser le mode Standard pour mon compteur Producteur)

Vu l'augmentation du volume de données remontée par la TIC en mode Standard,
vu l'augmentation de la vitesse de réception des données (9600 bits/s au lieu de 1200 bit/s),
vu la puissance des microcontroleurs cible pour implémenter cette librairie (ESP8266, ATMEGA328P,...)
vu l'utilisation que je fais de cette librairie,

j'ai choisi de supprimer le stockage des informations de TIC qui était fait dans l'ancienne librairie.

## Principes de fonctionnement
Après initialisation de la librairie en mode Standard ou Historique,
après initialisation des fonctions de callback (fin de Groupe EGR et/ou fin de Transmission ETX),
traiter chaque carracere recu (sur le port serie) avec .process(carractere_recu)

si on a reçu une fin de groupe, que le checksum est valide, les données reçues sont transmises via l'appel de la fonction de callback
>> c'est dans la fonction de callback que les données doivent etre traitées et éventuellement conservées par le programme principal
>> exemple : si j'ai recu le groupe "EAST", alors je conserve la valeur
si on a reçu ETX, on est notifie via la fonction de callback

## Installation
Copier le contenu de ce dossier (download zip) dans le dossier libraries de votre environnement Arduino Vous devriez avoir maintenant quelque chose comme `your_sketchbook_folder/libraries/LibTeleinfo` et ce dossier doit contentir les fichiers .cpp et .h ainsi que le sous dossier `examples`.
<br/>
Pour trouver votre dossier de sketchbook, dans l'environnement IDE, allez dans File>Preferences.
<br/>
allez voir ce [tutorial][2] sur les librairies Arduino si beoin.
<br/>

## Documentation
Pour la très bonne documentation et les très bons exemples je vous renvoie vers la librairie originale LibTeleinfo

[1]: https://hallard.me/libteleinfo


## Schémas electrique
A titre d'info j'utilise un seul ESP8266 pour lire mes 2 compteurs Linky
Les ports serie "logiciels" ne sont pas assez fiables, surtout quand la vitesse monte. J'utilise donc le seul port serie matériel de l'ESP8266 pour :
- en TX : emission des données de debug, toujours utile
- en RX : les 2 optocoupleurs (connectés aux 2 compteurs Linky) connectes sur la ligne RX  >> ??? on mélange les 2 signaux série ??? ça ne peut pas marcher ça !!
          En fait si, car la broche éméteur de chaque OPTO est branchée sur un GPIO différent (GPIO4 et GPIO5). En activant alternativement GPIO4 ou 5, RX ne va prendre que les signaux de l'un ou l'autre des OPTO donc compteurs.
          En mode debug il faut donc couper (switch) la ligne RX venant du PC.

## Exemple de traitement
Un des linky est en TIC Historique (CONSO), l'autre en mode Standard (Producteur). Le principe est donc le suivant :
Toutes les 10 minutes :
- activer le port serie en mode 1200 bauds
- reinitialiser la librairie en mode Histo
- activer GPIO4 (pour lire linky CONSO)
- lorsque j'ai lu les balises qui m'intéressent, désactiver GPIO4
- activer le port serie en mode 9600 bauds
- reinitialiser la librairie en mode Standard
- activer GPIO5 (pour lire linky PROD)
- lorsque j'ai lu les balises qui m'intéressent, désactiver GPIO5
- envoyer un rapport sur un serveur web
- dodo
