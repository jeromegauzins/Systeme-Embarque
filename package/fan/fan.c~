/*
* fan.c : 
*	Fonctions : 
*		d_open : appelée lors de l'ouverture du fichier virtuel lié au module
*		d_release : appelée lors de la fermeture du fichier virtuel lié au module
*		d_read : appelée lors de la lecture du fichier virtuel lié au module
*		d_write : appelée lors de l'écriture dans le fichier virtuel lié au module
*		fonctionInit : appelée lors du chargement du module, gère les initialisations
*		fonctionExit : appelée lors du déchargement du module, gère les libérations de mémoire
*/
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    int status;
    FILE *fdServo, *fdStick;
    int buffer[50];
    int angle;
    int vitesse = 0;
    
    fdServo = fopen("/dev/servo","w");
    fdStick = fopen("/dev/stick","r");
    
    if(!fdServo)
    {
        printf("Erreur ouverture fdServo\n");
        fclose(fdServo);
        return -1;
    }
    
    if(!fdStick)
    {
        printf("Erreur ouverture fdStick\n");
        fclose(fdStick);
        return -1;
    }
    

    angle = 0;
    while(1)
    {
        //on lit un entier représentatif de l'orientation du stick 
        status = fread(buffer,sizeof(int),1,fdStick);
        
        if(status < 0)
        {
            printf("Erreur lecture fread\n");
            fclose(fdServo);
            fclose(fdStick);
            return status;
        }
        
        if(buffer[0]< 1300)
        {
            //si on met le joytick vers l'avant, on determine un coef de vitesse en fonction de la position du joystick
            //coef de 0 a 10;
            vitesse = (1300-buffer[0])/130.0;
            angle += 1+vitesse/2;
        }else if(buffer[0] > 1700){
             //si on met le joytick vers l'arriere, on determine un coef de vitesse en fonction de la position du joystick
            //coef de 0 a 10;
            vitesse = (buffer[0]-1900)/220.0;
            angle -= (1+ vitesse/2);
        }else {
            //si le joystick est dans la zone du milieu, on fixe la vitesse à 0
            vitesse = 0;
        }
        
        printf("Buffer : |%d|\n",buffer[0]);
        printf("Vitesse : |%d|\n",vitesse);
        printf("Angle : |%d|\n",angle);
        
        //on incremente l'angle du servomoteur de vitesse
       
        
        //si on excede la borne du servo moteur, on le met a la borne
        if(angle>90) angle = 90;
        else if(angle<-90) angle =-90;
        
        //on ecrit l'angle dans le driver du servomoteur
        fprintf(fdServo,"%d",angle);
        fflush(fdServo);
        
        
        printf("Lu : %d\n",status);
        
        //on patiente 10 millisecondes
        usleep(10000);
    }
    return 0;
}
