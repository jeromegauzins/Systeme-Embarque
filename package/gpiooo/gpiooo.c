#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/timer.h>

/*
* gpiooo.c : ceci est un module dont le but est de s'entrainer à utiliser les gpio et un timer.
* 
* Fait clignoter une diode RGB en changeant de couleur toutes les 500ms
*
*	Fonctions :
*		red : initialise le gpio de la DEL rouge
*		green : initialise le gpio de la DEL verte
*		blue : initialise le gpio de la DEL bleue
*		freeRed : libere le gpio de la DEL rouge et la mémoire allouée pour elle
*		freeGreen : libere le gpio de la DEL verte et la mémoire allouée pour elle
*		freeBlue : libere le gpio de la DEL bleue et la mémoire allouée pour elle
*		timer_callback : gère l'alimentation des gpio, temporise les changements de couleurs de la diode
*		fonctionInit : appelée lors du chargement du module, gère les initialisations
*		fonctionExit : appelée lors du déchargement du module, gère les libérations de mémoire
*/


struct gpio *r,*g,*b; //GPIOs correspondant aux composantes rouge,bleue et verte de la DEL RGB.
int state; 
static struct timer_list timer;

static int red(int pinNo, int value);
static int green(int pinNo, int value);
static int blue(int pinNo, int value);
void freeRed(void);
void freeGreen(void);
void freeBlue(void);
void timer_callback(unsigned long data);
static int __init fonctionInit(void);
static void __exit fonctionExit(void);

/*
* Initialise la DEL rouge branchee sur la pin numerotee pinNo a value
*/
static int red(int pinNo, int value)
{
    

    int tmp = 0;
    r = (struct gpio*) kmalloc(sizeof(struct gpio),GFP_KERNEL);
    if(r == NULL)
    {
        printk(KERN_INFO"Erreur kmalloc red");
        return -1;
    }

    r->gpio = pinNo;
    tmp = gpio_request(r->gpio, "ROUGE");
    if(tmp < 0)
    {
        printk(KERN_INFO"Erreur gpio_request red");
        goto error_request_red;
    }

    tmp = gpio_direction_output(r->gpio, value);
    if(tmp < 0)
    {
        printk(KERN_INFO"Erreur gpio_direction_output red");
        goto error_direction_output_red;
    }

    return 0;
    
    error_direction_output_red:
        gpio_free(r->gpio);
    error_request_red:
        kfree(r);
    return tmp;
}

/*
* Initialise la DEL verte branchee sur la pin numerotee pinNo a value
*/
static int green(int pinNo, int value)
{
  
    int tmp = 0;
    g = (struct gpio*) kmalloc(sizeof(struct gpio),GFP_KERNEL);
    if(g == NULL)
    {
        printk(KERN_INFO"Erreur kmalloc green");
        return -1;
    }

    g->gpio = pinNo;
    tmp = gpio_request(g->gpio, "VERT");
    if(tmp < 0)
    {
        printk(KERN_INFO"Erreur gpio_request green");
        goto error_request_green;
    }

    tmp = gpio_direction_output(g->gpio, value);
    if(tmp < 0)
    {
        printk(KERN_INFO"Erreur gpio_direction_output green");
        goto error_direction_output_green;
    }
    return 0;
    
    error_direction_output_green:
        gpio_free(r->gpio);
    error_request_green:
        kfree(r);
    return tmp;
}

/*
* Initialise la DEL bleue branchee sur la pin numerotee pinNo a value
*/
static int blue(int pinNo, int value)
{
    int tmp = 0;
    b = (struct gpio*) kmalloc(sizeof(struct gpio),GFP_KERNEL);
    if(b == NULL)
    {
        printk(KERN_INFO"Erreur kmalloc blue");
        return -1;
    }

    b->gpio = pinNo;
    tmp = gpio_request(b->gpio, "BLUE");
    if(tmp < 0)
    {
        printk(KERN_INFO"Erreur gpio_request blue");
        goto error_request_blue;
    }

    tmp = gpio_direction_output(b->gpio, value);
    if(tmp < 0)
    {
        printk(KERN_INFO"Erreur gpio_direction_output blue");
        goto error_direction_output_blue;
    }
    return 0;
    
    error_direction_output_blue:
        gpio_free(r->gpio);
    error_request_blue:
        kfree(r);
    return tmp;
}

/*
* A chaque appel de la fonction, on passe a la combinaison de couleurs suivante
* Les DEL sont alimentees en out ou rien, et il y a sept couleurs differentes (le huitieme etat correspond a une DEL eteinte)
*/
void timer_callback(unsigned long data)
{
    int tmpState = state;
    if(state == 8)
    {
        state = 0;
        tmpState = 0;
    }

    gpio_set_value(r->gpio,state%2);
    state = (state - state%2)/ 2;

    gpio_set_value(g->gpio,state%2);
    state = (state - state%2)/ 2;

    gpio_set_value(b->gpio,state%2);
    state = tmpState + 1;

    //La couleur change toutes les 500ms.

    mod_timer(&timer, jiffies + msecs_to_jiffies(500));
}

/*
* Gère les initialisations lors du chargement:
*	- réserve et initialise des GPIOs
*	- initialise le timer
*/
static int __init fonctionInit(void)
{
    int tmp = 0;
    state = 0;
    
    //Reservation et initialisation des GPIOs ici

    tmp = red(4,1);
    if(tmp<0)
    {
        printk(KERN_INFO"Erreur ini red");

        return tmp;
    }
    
    tmp = green(2,1);
    if(tmp<0)
    {
        printk(KERN_INFO"Erreur ini green");
        goto error_green;
    }
    
    tmp = blue(1,1);
    if(tmp<0)
    {
        printk(KERN_INFO"Erreur ini blue");
        goto error_blue;
    }

    //Initialisation du timer :
    setup_timer(&timer,timer_callback,0);//mise en place du callback
    
    tmp = mod_timer(&timer, jiffies + msecs_to_jiffies(500));//le premier appel du timer sera dans 500ms
    if(tmp<0)
    {
         printk(KERN_INFO"Erreur mod_timer");
         fonctionExit();
	 return tmp;
    }
    
    return 0;
  
     error_blue:
        freeGreen();
     error_green:
        freeRed();
    return tmp;
}

/*
* Libere le gpio de la DEL rouge et la mémoire
*/
void freeRed(void)
{
    gpio_set_value(r->gpio,1);
    gpio_free(r->gpio);
    kfree(r);
}

/*
* Libere le gpio de la DEL vert et la mémoire
*/
void freeGreen(void)
{
    gpio_set_value(g->gpio,1);
    gpio_free(g->gpio);
    kfree(g);
}

/*
* Libere le gpio de la DEL bleue et la mémoire
*/
void freeBlue(void)
{
    gpio_set_value(b->gpio,1);
    gpio_free(b->gpio);
    kfree(b);
}

/*
* Gère les libérations de mémoire lors du déchargement:
*	- libère le timer
*	- libère les gpio
*/
static void __exit fonctionExit(void)
{
    //Liberation du timer :
    del_timer(&timer);

    //Liberation des gpios / memoire ici
    //On prend la peine d'eteindre les DELs avant
    freeRed();
    freeGreen();
    freeBlue();
}

/*
* Permet d'utiliser "fonctionInit" et "fonctionExit" respectivement lors du chargement et du déchargemetn du module
*/
module_init(fonctionInit);
module_exit(fonctionExit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Clement Dugue\nJerome Gauzins\nArthur Canal");
MODULE_DESCRIPTION("Utilisation GPIO pour controler une DEL RGB");
