/*
*   Servo.c :
*   Ce module sert a commander un servomoteur branché sur la pin22 de la carte 
*   et alimenté par une pile 5V indépendante.
*   L'angle est déterminé par la valeur contenue dans le fichier virtuel /dev/servo
*
*   Fonctions : 
*       d_open : appelée lors de l'ouverture du fichier virtuel lié au module
*   	d_release : appelée lors de la fermeture du fichier virtuel lié au module
*   	d_read : appelée lors de la lecture du fichier virtuel lié au module
*   	d_write : appelée lors de l'écriture dans le fichier virtuel lié au module
*   	fonctionInit : appelée lors du chargement du module, gère les initialisations
*   	fonctionExit : appelée lors du déchargement du module, gère les libérations de mémoire
*       initServo : appelée pour initialiser la gpio chargée du contrôle du servomoteur
*       freeServo : appelée pour libérer les objets liés à la gpio initialisée avec initServo
*/

#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/pwm.h>
#include <linux/uaccess.h>
#include <linux/fs.h>

//Initialisation du servo et libération de la mémoire
static int initServo(int pinNo, int value);
void freeServo(void);
//Chargement et déchargement du module
static int __init fonctionInit(void);
static void __exit fonctionExit(void);
//Fonctions d'interaction avec le fichier virtuel
static int d_open(struct inode *i, struct file *fp);
static int d_release(struct inode *i, struct file *fp);
static ssize_t d_read(struct file *fp, char __user *data, size_t size, loff_t *l);
static ssize_t d_write(struct file *fp, const char __user *data, size_t size, loff_t *l);

struct file_operations fops = {
    .read = d_read,
    .write = d_write,
    .open = d_open,
    .release = d_release
};

//Structures liées à la gpio
struct gpio *servo; //GPIO commandant la position du servomoteur
static struct pwm_device *pwmDevice;

//Valeurs initiales pour la pwm, en nanosecondes.
int pwmPeriod = 20000000;//Periode de chaque cycle complet de pwm
int dutyCycle = 2000000;//Durée du duty cycle par défaut
int major;//numero de major du peripherique

//Structures liées au fichier virtuel
struct device *dev;
static struct class *cls;
dev_t devt;

/*
* Affiche un message lors de l'ouverture du fichier virtuel associé au servomoteur
*/
static int d_open(struct inode *i, struct file *fp)
{
    printk(KERN_INFO"Le fichier a ete ouvert");
    return 0;
}

static int d_release(struct inode *i, struct file *fp)
{
    printk(KERN_INFO"Le fichier a ete ferme");
    return 0;
}

static ssize_t d_read(struct file *fp, char __user *data, size_t size, loff_t *l)
{
    char *buf = kmalloc(size, GFP_KERNEL);
	int i;
	for (i = 0; i < size; i ++)
		buf[i] = i;
	printk(KERN_INFO"Fichier lu\n");
	copy_to_user(data, buf, size);
	kfree(buf);
	return size;
}

/*
*   Recuperation de l'angle ecrit dans le fichier par l'utilisateur.
*       L'angle est donné en degrés, et entre -90 et 90. 
*       La traduction vers le dutyCycle correspondant est faite automatiquement.
*       Si la valeur entrée est hors des bornes prévues, on envoie un message d'erreur
*       et le dutyCycle reste inchangé. 
*/
static ssize_t d_write(struct file *fp, const char __user *data, size_t size, loff_t *l)
{//

    int val = 0;
    int errcode;

    //Reception du message
    unsigned long bytesCopied;
    char *msg = kmalloc(size + 1, GFP_KERNEL);
	bytesCopied = copy_from_user(msg, data, size);
    if(bytesCopied < 0)
    {
        printk(KERN_INFO"Erreur au copy_from_user");        
        kfree(msg);
        return bytesCopied;
    }	
    msg[size] = 0;

    //Passage d'un message a un angle
    errcode = kstrtoint(msg,10,&val);    
    if(errcode<0)
    {   
        printk(KERN_INFO"Erreur au kstrtoint");
        kfree(msg);
        return errcode;
    }
   
    val = val*1770000/180 + 1385000;//equivalence angle vers dutyCycle

    if(val > 2270000 || val < 500000)
    {//dutyCycle trop court ou trop long : le servo ne peut pas depasser -90° ni 90°
        printk(KERN_INFO "Valeur hors des bornes prevues, pas de changement\n");
    }
    else
    {
        dutyCycle = val;
        pwm_disable(pwmDevice);
        pwm_config(pwmDevice,dutyCycle,pwmPeriod);
        pwm_enable(pwmDevice);
    }
    kfree(msg);
	return size;
}

static int initServo(int pinNo, int value)
{    
    //Initialise le gpio commandant le servo
    int tmp = 0;
    servo = (struct gpio*) kmalloc(sizeof(struct gpio),GFP_KERNEL);
    if(servo == NULL)
    {
        printk(KERN_INFO"Erreur kmalloc servo");
        return -1;
    }

    servo->gpio = pinNo;
    tmp = gpio_request(servo->gpio, "SERVO");
    if(tmp < 0)
    {
        printk(KERN_INFO"Erreur gpio_request servo");
        goto error_request_servo;

    }

    tmp = gpio_direction_output(servo->gpio, value);
    if(tmp < 0)
    {
        printk(KERN_INFO"Erreur gpio_direction_output servo");
        goto error_direction_output_servo;
    }

    return 0;
    
    error_direction_output_servo:
        gpio_free(servo->gpio);
    error_request_servo:
        kfree(servo);
    return tmp;
}

static int __init fonctionInit(void)
{
    int tmp;
    int status;

    //On utilise la pin 22, qui correspond au canal PWM2
    tmp = initServo(22,1);
    if(tmp<0)
    {
        printk(KERN_INFO"Erreur ini servo");
        return tmp;
    }

    major = register_chrdev(0,"SERVO",&fops);
    if(major < 0)
    {
        printk(KERN_INFO"Erreur register_chrdev");
        status = major;
        goto erreurRegister;
    }

    cls = class_create(THIS_MODULE, "servo Module");
    if(IS_ERR(cls))
    {
        printk(KERN_INFO"Erreur register_chrdev");
        status = PTR_ERR(cls);
        goto erreurClass;
    }
    
    devt = MKDEV(major,0);
    dev = device_create(cls,NULL,devt,NULL,"servo");
    
    status = IS_ERR(dev) ? PTR_ERR(dev) : 0;

    if(status != 0)
    {
        printk(KERN_INFO"Erreur register_chrdev");

        goto erreurDevice;
    }

    //Initialisation de la pwm (notre branchement utilise le canal PWM2)
    pwmDevice = pwm_request(2,"SERVO_PWM");
    pwm_config(pwmDevice,dutyCycle,pwmPeriod);
    pwm_enable(pwmDevice);
    
    return 0;

erreurDevice:
    class_destroy(cls);
erreurClass:
    unregister_chrdev(major,"SERVO");
erreurRegister:
    gpio_free(servo->gpio);
    kfree(servo);
    return status;
}

void freeServo(void)
{
    gpio_free(servo->gpio);
    kfree(servo);
}


static void __exit fonctionExit(void)
{
    device_destroy(cls,devt);
    class_destroy(cls);
    unregister_chrdev(major,"SERVO");
    freeServo();
    pwm_disable(pwmDevice);
    pwm_free(pwmDevice);
}

module_init(fonctionInit);
module_exit(fonctionExit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Clement Dugue\nJerome Gauzins\nArthur Canal");
MODULE_DESCRIPTION("Utilisation GPIO pour commander un servomoteur");
