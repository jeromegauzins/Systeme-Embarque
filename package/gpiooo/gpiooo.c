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
        return -1;
    }
    r->gpio = pinNo;
    tmp = gpio_request(r->gpio, "VERT");
    if(tmp < 0)
    {
        return tmp;
    }
    tmp = gpio_direction_output(r->gpio, value);
    if(tmp < 0)
    {
        return tmp;
    }
    return 0;
}

static int green(int pinNo, int value)
{
  //Initialise la DEL verte branchee sur la pin numerotee pinNo a value
    int tmp = 0;
    g = (struct gpio*) kmalloc(sizeof(struct gpio),GFP_KERNEL);
    if(g == NULL)
    {
        return -1;
    }
    g->gpio = pinNo;
    tmp = gpio_request(g->gpio, "VERT");
    if(tmp < 0)
    {
        return tmp;
    }
    tmp = gpio_direction_output(g->gpio, value);
      if(tmp < 0)
    {
        return tmp;
    }
    return 0;
}

static int blue(int pinNo, int value)
{
    //Initialise la DEL bleue branchee sur la pin numerotee pinNo a value
    int tmp = 0;
    b = (struct gpio*) kmalloc(sizeof(struct gpio),GFP_KERNEL);
    if(b == NULL)
    {
        return -1;
    }
    b->gpio = pinNo;
    tmp = gpio_request(b->gpio, "BLUE");
    if(tmp < 0)
    {
        return tmp;
    }
    tmp = gpio_direction_output(b->gpio, value);
    if(tmp < 0)
    {
        return tmp;
    }
    return 0;
}

void timer_callback(unsigned long data)
{//On boucle sur toutes les combinaisons de couleurs (avec intensite maximale)
    int tmpState = state;
    if(state == 8)
    {
        state = 0;
        tmpState = 0;
    }
    red(r->gpio,state%2);
    state = (state - state%2)/ 2;
    green(g->gpio,state%2);
    state = (state - state%2)/ 2;
    blue(b->gpio,state%2);
    state = tmpState + 1;
    //Puis on rajoute 500ms au timer.
    printk(KERN_INFO"timer");
    mod_timer(&t, jiffies + msecs_to_jiffies(500));
}

static int __init fonctionInit(void)
{
    state = 0;
    //Reservation et initialisation des GPIOs ici
    red(4,1);
    green(2,1);
    blue(1,1);

    //Initialisation du timer :
    setup_timer(&t,timer_callback,0);//mise en place du callback
    mod_timer(&t, jiffies + msecs_to_jiffies(500));//le timer sera appele dans 500ms
    
    return 0;
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

