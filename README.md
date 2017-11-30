# Systeme-Embarque

**Systeme-Embarque - Réalisation d'un Servomoteur contrôlé par un Joystick**

### Auteurs
 - CANAL Arthur
 - DUGUE Clément
 - GAUZIN Jérôme

### Vidéo
 - *cf*: MOV_20171117_1848424.mp4 dans dossier "doc"

### Matériel 
*(voir documentation pour chaque matériel dans le dossier "doc")
 - Carte iMX233-OLINUXINO-MAXI
 - Servomoteur SG90Servo
 - Joystick 5V
 - ADC MCP3201


### Comment utiliser *Systeme-Embarque*

### Réaliser les branchements (sans oublier le port série)
 - *cf*: les différents schémas fournis dans dossier "doc"(branchements sur la plaquette, raccord avec la carte...)

### Se procurer buildroot et le préparer :
 - Télécharger buildroot et l'installer
 - Dans le dossier `buildroot`, lancer la commande `make olimex_imx233_olinuxino_defconfig`

#### installer les drivers et programmes :
 - Se placer dans le dossier `buildroot`
 - Coller le contenu du dossier `package` du projet dans le dossier package de `buildroot`
 - Remplacer les "config.in" de `buildroot` et de `package` par ceux du projet
 - Remplacer le fichier "imx23-olinuxino.dts" dans le dossier `buildroot/output/build/linux-4.9.49/arch/arm/boot/dts/`
 - Faire un make menuconfig et ajouter les drivers voulus dans l'option `Target Packages/Systeme Embarque`
 - Cocher l'option "Build a device Tree Blob" dans `Kernel` en spécifiant le bon chemin pour le fichier dts
 - Cocher "spi-tools" dans `Target Packages/Hardware Handling`
 - Au besoin, ajouter un mot de passe root
 - Confirmer les choix, sortir de menuconfig
 - Compiler avec `make`
 - Flasher avec `flash.sh` (/!\ ATTENTION : ce script flashe sur sdb sans demander de confirmation)
 - OU démonter le lecteur correspondant à la carte microSD (noté <SDX> par la suite) puis flasher avec `sudo dd if=output/images/sdcard.img of=/dev/<SDX> bs=50M count=1`

#### utiliser les drivers :
 - Se connecter à la carte olimex *via* la liaison série
 - Insérer la carte microSD
 - Redémarer la carte iMX233
 - Après le chargement de la carte mémoire, rentrer votre login puis votre mot de passe
 - Rentrer `modprobe servo`
 - Rentrer `modprobe stick`
 - Rentrer `fan`
 - Le ventilateur répond maintenant aux déplacements du stick sur l'axe x
 - CTRL + C pour arrêter
 - Rentrer `rmmod servo`
 - Rentrer `rmmod stick`
 - Rentrer `poweroff`pour éteindre la carte

