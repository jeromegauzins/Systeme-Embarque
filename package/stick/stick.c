#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/spi/spi.h>

static int d_open(struct inode *i, struct file *fp);
static int d_release(struct inode *i, struct file *fp);
static ssize_t d_read(struct file *fp, char __user *data, size_t size, loff_t *l);
static ssize_t d_write(struct file *fp, const char __user *data, size_t size, loff_t *l);
static int probe_spi(struct spi_device *spi);
static int remove_spi(struct spi_device *spi);
static int __init fonctionInit(void);
static void __exit fonctionExit(void);

struct gpio *servo;
int state,major;
int valueStick;
static struct spi_driver spiDriver  = {
        .driver = {
            .name   = "stick",
            .owner  = THIS_MODULE
        },
        .probe  = probe_spi,
        .remove = remove_spi
};

char *bufSpi;
struct device *stickDevice;
static struct class *c;
dev_t devt;
int res;

struct file_operations fops = {
    .read = d_read,
    .write = d_write,
    .open = d_open,
    .release = d_release
};

static int probe_spi(struct spi_device *arg_spiDevice){
    char *bufferSpi = kcalloc(3,sizeof(char),GFP_KERNEL);  
    int val = 0;
    int i;    
    //int erreur = 0;
    printk(KERN_INFO"probe_spi\n");
    
    arg_spiDevice->bits_per_word = 8;
    spi_setup(arg_spiDevice);
    spi_read(arg_spiDevice,bufferSpi,3);

    
    for(i=0;i<3;i++){
        printk(KERN_INFO"Buffer spi %d : %x\n",i,bufferSpi[i]);
    }

    printk("mask = %x",(bufferSpi[0] & 0x1F));
    printk("mask decale= %x",(bufferSpi[0] &  0x1F)<<7);
    printk("mask = %x",(bufferSpi[1] & 0xFE));
    printk("mask decale= %x",(bufferSpi[1] & 0xFE)>>1);
    val = ((bufferSpi[0] & 0x1F)<<7) + ((bufferSpi[1] & 0xFE)>>1);
    printk("val: %d\n",val);

    //
    if(val> 3000){
        printk("bas\n");
    }
    else if(val< 1000){
        printk("haut\n");
    }
    else{
        printk("milieu\n");
    }

    valueStick = val;

    /*erreur = kstrtoint(bufferSpi,10,&res);
    if(erreur<0){
        kfree(bufferSpi);
        printk(KERN_INFO"Erreur au kstrtoint %d", erreur);
        return erreur;    
    }

    printk(KERN_INFO"coucou : %d\n",res);*/
    kfree(bufferSpi);

    return 0;
}
static int remove_spi(struct spi_device *spi){
    printk("remove_spi\n");
    return 0;
}

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
    char *buf = kcalloc(size,sizeof(char),GFP_KERNEL);  
	int error;
    printk("size = %d\n",size);
    printk("sizeof = %d\n",sizeof(valueStick));

    if(size < sizeof(valueStick))
    {
        kfree(buf);
	    printk(KERN_ERR"Error size must be >= %d\n",sizeof(valueStick));
        return -1;
    }

	printk(KERN_INFO"\nFichier lu\n");
	error = copy_to_user(data, &valueStick, sizeof(valueStick));
    if(error <0)
    {
        kfree(buf);
	    printk(KERN_ERR"Error dans copytouser : %d\n",error);
        return error;
    }
	kfree(buf);
	return sizeof(valueStick);
}

static ssize_t d_write(struct file *fp, const char __user *data, size_t size, loff_t *l)
{

	return size;
}

static int __init fonctionInit(void)
{
    int status;
    // inscription dans la liste des pilotes ( /proc/devices )
    //*  
    major = register_chrdev(0,"STICK",&fops);
    if(major < 0)
    {
        printk(KERN_INFO"Erreur register_chrdev");
        status = major;
        goto erreurRegister;
    }

    // creation d'une structure "class" pour pouvoir utiliser la fonction "create" après
    c = class_create(THIS_MODULE, "stickModule");
    if(IS_ERR(c))
    {
        printk(KERN_INFO"Erreur register_chrdev");
        status = PTR_ERR(c);
        goto erreurClass;
    }
    
    // macro initilisant le device
    devt = MKDEV(major,0);

    stickDevice = device_create(c,NULL,devt,NULL,"stickDevice");
    

    // on vérifie qu'il n'y a pas stickDevice'erreur à la création
    status = IS_ERR(stickDevice) ? PTR_ERR(stickDevice) : 0;
    if(status != 0)
    {
        printk(KERN_INFO"Erreur device_create");

        goto erreurDevice;
    }
    //*/

    //Partie SPI
    //*

    printk("ON REGISTER LE DRIVER (avant)\n");
    status = spi_register_driver(&spiDriver);
    if(status <0)
    {
        printk("spi_registrer driver failed !!!!!!!!!!!!!!!!!!!!!! %d\n",status);
        goto erreurSPI;
    }
    
    return 0;

// gestion des erreurs 
erreurSPI:
    spi_unregister_driver(&spiDriver);
erreurDevice:
    class_destroy(c);
erreurClass:
    unregister_chrdev(major,"stick");
erreurRegister:
    return status;
}


static void __exit fonctionExit(void)
{
    spi_unregister_driver(&spiDriver);
    device_destroy(c,devt);
    class_destroy(c);
    unregister_chrdev(major,"stick");
}

module_init(fonctionInit);
module_exit(fonctionExit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Clement Dugue\nJerome Gauzins\nArthur Canal");
MODULE_DESCRIPTION("Driver pour le stick en SPI");
