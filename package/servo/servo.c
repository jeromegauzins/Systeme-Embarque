#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/pwm.h>
#include <linux/uaccess.h>
#include <linux/fs.h>

struct gpio *servo;
int state;
static struct pwm_device *p;
int pwmPeriod = 20000000;//en ns
int dutyCycle = 2000000;

int major;

struct device *d;
static struct class *c;
dev_t devt;

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

static int initServo(int pinNo, int value);
void freeServo(void);
static int __init fonctionInit(void);
static void __exit fonctionExit(void);

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
	printk(KERN_INFO"Read!!!!\n");
	//copy_to_user(data, buf, size);
	kfree(buf);
	return size;
}

static ssize_t d_write(struct file *fp, const char __user *data, size_t size, loff_t *l)
{
    int val = 0;
    int errcode;
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
	printk(KERN_INFO "J'ai reçu ça: %s\n", msg);
    errcode = kstrtoint(msg,10,&val);    
    if(errcode<0)
    {   
        printk(KERN_INFO"Erreur au kstrtoint");
        kfree(msg);
        return errcode;
    }
    val = val*1770000/180 + 1385000;//equivalence angle vers dutyCycle
    printk(KERN_INFO "Apres strtol: %d\n", val);
    if(val > 2270000 || val < 500000)//dutyCycle trop court ou trop long
    {
        printk(KERN_INFO "Valeur trop basse, pas de changement\n");
    }
    else
    {
        dutyCycle = val;
        pwm_disable(p);
        pwm_config(p,dutyCycle,pwmPeriod);
        pwm_enable(p);
    }
    kfree(msg);
	return size;
}

static int initServo(int pinNo, int value)
{
    
    //Initialise la DEL rouge branchee sur la pin numerotee pinNo a value
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

    tmp = initServo(22,1);
    if(tmp<0)
    {
        printk(KERN_INFO"Erreur ini servo");
        return tmp;
    }

    major = register_chrdev(0,"SERVO",&fops);//Fichier dans /dev/devices ?
    if(major < 0)
    {
        printk(KERN_INFO"Erreur register_chrdev");
        status = major;
        goto erreurRegister;
    }

    c = class_create(THIS_MODULE, "servo Module");
    if(IS_ERR(c))
    {
        printk(KERN_INFO"Erreur register_chrdev");
        status = PTR_ERR(c);
        goto erreurClass;
    }
    
    devt = MKDEV(major,0);
    d = device_create(c,NULL,devt,NULL,"servo");
    
    status = IS_ERR(d) ? PTR_ERR(d) : 0;

    if(status != 0)
    {
        printk(KERN_INFO"Erreur register_chrdev");

        goto erreurDevice;
    }

    p = pwm_request(2,"SERVO_PWM");
    pwm_config(p,dutyCycle,pwmPeriod);
    pwm_enable(p);
    
    return 0;

erreurDevice:
    class_destroy(c);
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
    device_destroy(c,devt);
    class_destroy(c);
    unregister_chrdev(major,"SERVO");
    freeServo();
    pwm_disable(p);
    pwm_free(p);
}

module_init(fonctionInit);
module_exit(fonctionExit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Clement Dugue\nJerome Gauzins\nArthur Canal");
MODULE_DESCRIPTION("Utilisation GPIO pour commander un servomoteur");
