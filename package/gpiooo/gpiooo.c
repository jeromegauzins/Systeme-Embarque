#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/timer.h>

struct gpio *r,*g,*b;
int state;
static struct timer_list t;

static int red(int pinNo, int value);
static int green(int pinNo, int value);
static int blue(int pinNo, int value);
void freeRed(void);
void freeGreen(void);
void freeBlue(void);
void timer_callback(unsigned long data);
static int __init fonctionInit(void);
static void __exit fonctionExit(void);

irqreturn_t irq_bouton (int irq, void *data)
{
    return IRQ_HANDLED;
}

static int red(int pinNo, int value)
{
    
    //Initialise la DEL rouge branchee sur la pin numerotee pinNo a value
    int tmp = 0;
    r = (struct gpio*) kmalloc(sizeof(struct gpio),GFP_KERNEL);
    if(r == NULL)
    {
        printk(KERN_INFO"Erreur kmalloc red");
        return -1;
    }
    r->gpio = pinNo;
    tmp = gpio_request(r->gpio, "VERT");
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


static int green(int pinNo, int value)
{
  //Initialise la DEL verte branchee sur la pin numerotee pinNo a value
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

static int blue(int pinNo, int value)
{
    //Initialise la DEL bleue branchee sur la pin numerotee pinNo a value
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

void timer_callback(unsigned long data)
{//On boucle sur toutes les combinaisons de couleurs (avec intensite maximale)
    int tmpState = state;
    if(state == 8)
    {
        state = 0;
        tmpState = 0;
    }
    //red(r->gpio,state%2);
    gpio_set_value(r->gpio,state%2);
    state = (state - state%2)/ 2;
    //green(g->gpio,state%2);
    gpio_set_value(g->gpio,state%2);
    state = (state - state%2)/ 2;
    //blue(b->gpio,state%2);
    gpio_set_value(b->gpio,state%2);
    state = tmpState + 1;
    //Puis on rajoute 500ms au timer.
    //printk(KERN_INFO"timer, state = %d\n",state);
    mod_timer(&t, jiffies + msecs_to_jiffies(500));
}

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
    setup_timer(&t,timer_callback,0);//mise en place du callback
    
    tmp = mod_timer(&t, jiffies + msecs_to_jiffies(500));//le timer sera appele dans 500ms
    if(tmp<0)
    {
         printk(KERN_INFO"Erreur mod_timer");
         fonctionExit();
    }
    
    return 0;

  
     error_blue:
        freeGreen();
     error_green:
        freeRed();
    return tmp;
}

void freeRed(void)
{
    gpio_set_value(r->gpio,1);
    gpio_free(r->gpio);
    kfree(r);
}

void freeGreen(void)
{
    gpio_set_value(g->gpio,1);
    gpio_free(g->gpio);
    kfree(g);
}

void freeBlue(void)
{
    gpio_set_value(b->gpio,1);
    gpio_free(b->gpio);
    kfree(b);
}

static void __exit fonctionExit(void)
{
    //Liberation du timer :
    del_timer(&t);

    //Liberation des gpios / memoire ici
    //On prend la peine d'eteindre les DELs avant
    freeRed();
    freeGreen();
    freeBlue();
}

module_init(fonctionInit);
module_exit(fonctionExit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Clement Dugue\nJerome Gauzins\nArthur Canal");
MODULE_DESCRIPTION("Utilisation GPIO");

