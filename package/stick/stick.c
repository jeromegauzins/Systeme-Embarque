/*
*   stick.c : module de récupération de l'angle du stick analogique
*       On y récupère la valeur passée par le CAN (via le  bus SPI),
*       qu'on stocke dans un fichier virtuel.
*   
*   Fonctions : 
*		d_open : appelée lors de l'ouverture du fichier virtuel lié au module
*		d_release : appelée lors de la fermeture du fichier virtuel lié au module
*		d_read : appelée lors de la lecture du fichier virtuel lié au module
*		d_write : appelée lors de l'écriture dans le fichier virtuel lié au module
*		fonctionInit : appelée lors du chargement du module, gère les initialisations
*		fonctionExit : appelée lors du déchargement du module, gère les libérations de mémoire
*       probe_spi : appelée lors de la recherche de périphériques branchés sur le bus SPI
*       remove_spi : jamais appelée ici normalement
*       readValue : appelée pour obtenir l'orientation du stick
*/

#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/spi/spi.h>

//Fonctions de callback du fichier virtuel
static int d_open(struct inode *i, struct file *fp);
static int d_release(struct inode *i, struct file *fp);
static ssize_t d_read(struct file *fp, char __user *data, size_t size, loff_t *l);
static ssize_t d_write(struct file *fp, const char __user *data, size_t size, loff_t *l);

//Fonctions liées au fonctionnement du bus SPI
static int probe_spi(struct spi_device *spi);
static int remove_spi(struct spi_device *spi);
static int readValue(void);

//Chargement et déchargement du module
static int __init fonctionInit(void);
static void __exit fonctionExit(void);


struct gpio *servo;
int state,major;

static struct spi_device* spiDevice;
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


/*
*   probe_spi : Configure le bus SPI et lit la première valeur du stick
*/
static int probe_spi(struct spi_device *arg_spiDevice)
{
    int status = 0;
    arg_spiDevice->bits_per_word = 8;
    status = spi_setup(arg_spiDevice);
    if(status < 0)
    {
        printk(KERN_ERR"Erreur spi_setup : %d\n",status);
        return status;
    }

    spiDevice = arg_spiDevice;

    return status;
}

/*
*   remove_spi : théoriquement inutile dans notre cas d'utilisation
*/
static int remove_spi(struct spi_device *spi)
{
    printk("remove_spi\n");
    return 0;
}


/*
*   d_open : Affiche un message lorsque le fichier virtuel est ouvert
*/
static int d_open(struct inode *i, struct file *fp)
{
    printk(KERN_INFO"Le fichier stickDevice a ete ouvert\n");
    return 0;
}

/*
*   d_release : Affiche un message lorsque le fichier virtuel est fermé
*/
static int d_release(struct inode *i, struct file *fp)
{
    printk(KERN_INFO"Le fichier stickDevice a ete ferme\n");
    return 0;
}

/*
*   d_read : Récupère et transmet la valeur du stick dans data lorsque l'utilisateur lit dans le fichier virtuel
*/
static ssize_t d_read(struct file *fp, char __user *data, size_t size, loff_t *l)
{
	int error,valueStick;
    char *buf = kcalloc(size,sizeof(char),GFP_KERNEL); 
    if(buf < 0)
    {
        printk(KERN_ERR"Erreur kcalloc du tampon : %d\n",(int)buf);
        return buf;
    }

    if(size < sizeof(valueStick))
    {
        kfree(buf);
	    printk(KERN_ERR"Error size must be >= %d\n",sizeof(valueStick));
        return -1;
    }
    
    valueStick = readValue();
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

/*
*   d_write : Affiche un message lorsque l'utilisateur essaye d'écrire dans le fichier virtuel
*/
static ssize_t d_write(struct file *fp, const char __user *data, size_t size, loff_t *l)
{
    printk(KERN_INFO"Tentative d'ecriture dans le fichier\n");
	return size;
}

/*
*   readValue : Lit et renvoie la valeur du stick
*/
static int readValue(void){

    int val = 0;  
    char *bufferSpi = kcalloc(3,sizeof(char),GFP_KERNEL);  
    if(bufferSpi < 0)
    {
        printk(KERN_ERR"Erreur au kcalloc du tampon : %d\n",(int)bufferSpi);
        return bufferSpi;
    }

    val = spi_read(spiDevice,bufferSpi,3);
    if(val < 0)
    {
        printk(KERN_ERR"Erreur a spi_read : %d\n",val);
        kfree(bufferSpi);
        return val;
    }

    val = ((bufferSpi[0] & 0x1F)<<7) + ((bufferSpi[1] & 0xFE)>>1);
    kfree(bufferSpi);

    return val;
}

/*
*   fonction_Init : On réserve la mémoire et on initialise les variables dont on a besoin.
*/
static int __init fonctionInit(void)
{
    int status;
    // inscription dans la liste des pilotes ( /proc/devices )
    major = register_chrdev(0,"stick",&fops);
    if(major < 0)
    {
        printk(KERN_INFO"Erreur register_chrdev\n");
        status = major;
        goto erreurRegister;
    }

    // creation d'une structure "class" pour pouvoir utiliser la fonction "create" après
    c = class_create(THIS_MODULE, "stickModule");
    if(IS_ERR(c))
    {
        printk(KERN_INFO"Erreur register_chrdev\n");
        status = PTR_ERR(c);
        goto erreurClass;
    }
    
    // macro initilisant le device
    devt = MKDEV(major,0);

    stickDevice = device_create(c,NULL,devt,NULL,"stick");

    // on vérifie qu'il n'y a pas stickDevice'erreur à la création
    status = IS_ERR(stickDevice) ? PTR_ERR(stickDevice) : 0;
    if(status != 0)
    {
        printk(KERN_INFO"Erreur device_create");
        goto erreurDevice;
    }

    //Partie SPI
    status = spi_register_driver(&spiDriver);
    if(status <0)
    {
        printk("spi_registrer driver failed, %d\n",status);
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

/*
*   fonctionExit : On libère la mémoire lorsque le module est déchargé.
*/
static void __exit fonctionExit(void)
{
    spi_unregister_driver(&spiDriver);
    device_destroy(c,devt);
    class_destroy(c);
    unregister_chrdev(major,"stick");
}

/*
* Permet d'utiliser "fonctionInit" et "fonctionExit" respectivement lors du chargement et du déchargement du module
*/
module_init(fonctionInit);
module_exit(fonctionExit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Clement Dugue\nJerome Gauzins\nArthur Canal");
MODULE_DESCRIPTION("Driver pour le stick en SPI");
