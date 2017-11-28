# Systeme-Embarque

**Systeme-Embarque - Réalisation d'un Servomoteur controllé par un Joystick**

### Auteurs
 - CANAL Arthur
 - DUGUE Clément
 - GAUZIN Jérome

### Matériel 
*(voir documentation pour chaque matériel dans le dossier "doc")
 - Carte iMX233-OLINUXINO-MAXI
 - Seromoteur SG90Servo
 - Joystick 5V
 - Kit ADC


### Comment utiliser *Systeme-Embarque*

#### faudrait explique comment avoir buildroot etc...

#### installer les driver et user-programme:
 - ce placer dans le dossier `buildroot`
 - coller les dossiers présent dans le dossier `package` du projet dans le dossier package de `buildroot`
 - remplacer les "config.in" de `buildroot` et de `package` par ceux du projet
 - remplacer le fichier "imx23-olinuxino.dts" dans le dossier `buildroot/je/sais/plus/mais/c'est/long/`
 - faire un make menuconfig et ajouter les drivers dans l'onglet `...`

 - compiler
 - flasher

#### utiliser les drivers :
 - ouvrir un putty (faudrait plus d'explications)
 - mettre la carte mémoire dans la carte iMX233
 - redémarer la carte iMX233
 - après le chargement de la carte mémoire, rentrer votre login puis votre mot de passe
 - rentrer `modprobe servo`
 - rentrer `modprobe stick`
 - rentrer `fan`
 - amusez vous !
 - CTRL + C pour arrêter
 - rentrer `rmmod servo`
 - rentrer `rmmod stick`

