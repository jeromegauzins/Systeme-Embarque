# Systeme-Embarque

**Systeme-Embarque - Réalisation d'un Servomoteur controllé par un Joystick**

### Auteurs
 - CANAL Arthur
 - DUGUE Clément
 - GAUZIN Jérome

### Matériel 
*(voir documentation pour chaque matériel dans le dossier "doc")
 - Carte iMX233-OLINUXINO-MAXI
 - Servomoteur SG90Servo
 - Joystick 5V
 - ADC MCP3201


### Comment utiliser *Systeme-Embarque*

### Réaliser les branchements (sans oublier le port série)
 - *cf* les différents schémas fournis (branchements sur la plaquette, raccord avec la carte...)

### Se procurer buildroot

#### installer les drivers et programmes :
 - se placer dans le dossier `buildroot`
 - coller les dossiers présent dans le dossier `package` du projet dans le dossier package de `buildroot`
 - remplacer les "config.in" de `buildroot` et de `package` par ceux du projet
 - remplacer le fichier "imx23-olinuxino.dts" dans le dossier `buildroot/output/build/linux-4.9.49/arch/arm/boot/dts`
 - faire un make menuconfig et ajouter les drivers voulus dans l'option `Target Packages/Systeme Embarque`
 - Cocher l'option "Use a device Tree Blob" dans `Kernel` en spécifiant le bon chemin pour le fichier dts
 - Cocher "spi-tools" dans `Target Packages/Hardware Handling`
 - Au besoin, ajouter un mot de passe root
 - Confirmer les choix, sortir de menuconfig
 - compiler avec `make`
 - flasher avec `flash.sh` (ATTENTION : ce script flashe sur sdb sans demander de confirmation)
 - OU démonter le lecteur correspondant à la carte microSD (noté <SDX> par la suite) puis flasher avec `sudo dd if=output/images/sdcard.img of=/dev/<SDX> bs=50M count=1`

#### utiliser les drivers :
 - se connecter à la carte olimex *via* la liaison série
 - insérer la carte microSD
 - redémarer la carte iMX233
 - après le chargement de la carte mémoire, rentrer votre login puis votre mot de passe
 - rentrer `modprobe servo`
 - rentrer `modprobe stick`
 - rentrer `fan`
 - Le ventilateur répond maintenant aux déplacements du stick sur l'axe x
 - CTRL + C pour arrêter
 - rentrer `rmmod servo`
 - rentrer `rmmod stick`

