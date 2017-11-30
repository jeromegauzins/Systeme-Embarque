/*
* fan.c : 
*	Programme utilisateur faisant le lien entre le driver du stick et le driver du servomoteur
*   Récupère les valeurs du stick et les traduit en valeur d'angle avant de les envoyer au servomoteur.
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
    
    //On ouvre les fichiers virtuels correspondant respectivement au servomoteur et au joystick
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
            goto error;
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
               
        
        //si on excede la borne du servo moteur, on le met a la borne la plus proche
        if(angle>90) angle = 90;
        else if(angle<-90) angle =-90;
        
        //on transmet l'angle au driver du servomoteur
        status = fprintf(fdServo,"%d",angle);
        if(status < 0)
        {
            printf("Erreur a l'ecriture dans fdServo : %d\n",status);
            goto error;
        }
        fflush(fdServo);
        
        
        printf("Lu : %d\n",status);
        
        //on patiente 10 millisecondes
        usleep(10000);
    }
    status = 0;
error:
    fclose(fdServo);
    fclose(fdStick);
    return status;
}
