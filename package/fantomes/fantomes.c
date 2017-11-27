/*
* fantomes.c : ceci est un module dont le but est de
*	s'entrainer à utiliser les fichiers virtuels.
*   Affiche des messages correspondant aux interactions entre l'utilisateur et le fichier virtuel.
*
*	Fonctions : 
*		d_open : appelée lors de l'ouverture du fichier virtuel lié au module
*		d_release : appelée lors de la fermeture du fichier virtuel lié au module
*		d_read : appelée lors de la lecture du fichier virtuel lié au module
*		d_write : appelée lors de l'écriture dans le fichier virtuel lié au module
*		fonctionInit : appelée lors du chargement du module, gère les initialisations
*		fonctionExit : appelée lors du déchargement du module, gère les libérations de mémoire
*/
#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/fs.h>


static int __init fonctionInit(void);
static void __exit fonctionExit(void);

//Fonctions de callback
static int d_open(struct inode *i, struct file *fp);
static int d_release(struct inode *i, struct file *fp);
static ssize_t d_read(struct file *fp, char __user *data, size_t size, loff_t *l);
static ssize_t d_write(struct file *fp, const char __user *data, size_t size, loff_t *l);

int major;
struct device *dev;
static struct class *cls;
dev_t devt;
struct file_operations fops = {
    .read = d_read,
    .write = d_write,
    .open = d_open,
    .release = d_release
};


/*
*   d_open : Affiche un message lorsque le fichier virtuel est ouvert
*/
static int d_open(struct inode *i, struct file *fp)
{
    printk(KERN_INFO"Le fichier a ete ouvert\n");
    return 0;
}

/*
*   d_release : affiche un message lorsque le fichier virtuel est fermé
*/
static int d_release(struct inode *i, struct file *fp)
{
    printk(KERN_INFO"Le fichier a ete ferme\n");
    return 0;
}

/*
*   d_read : affiche un message lorsque l'on lit dans le fichier virtuel
*/
static ssize_t d_read(struct file *fp, char __user *data, size_t size, loff_t *l)
{
	printk(KERN_INFO"Read!!!!\n");
	return size;
}

/*
*   d_write : affiche un message lorsque l'on écrit dans le fichier virtuel,
*             et récupère les données écrites dans le fichier (on se contente alors de les afficher)
*/
static ssize_t d_write(struct file *fp, const char __user *data, size_t size, loff_t *l)
{
    int errCode = 0;
    
    char *msg = kmalloc(size + 1, GFP_KERNEL);
    if(msg < 0)
    {
        printk(KERN_ERR"Erreur kmalloc du tampon d'ecriture : %d\n",msg);
        return msg;
    }
    
    errCode = copy_from_user(msg, data, size);
	if(errCode != 0)
	{
	    printk(KERN_ERR"Erreur copy_from_user : %d\n",errCode);
	    kfree(msg);
	    return errCode;
	}
	
	msg[size] = 0;

	printk(KERN_INFO "Message recu : %s\n", msg);
	kfree(msg);
	return size;
}

/*
*   fonction_Init : On réserve la mémoire et on initialise les variables dont on a besoin.
*/
static int __init fonctionInit(void)
{
    int status;
    //Initialisation du character device
    major = register_chrdev(0,"charDevice",&fops);
    if(major < 0)
    {
        status = major;
        printk(KERN_ERR"Erreur register_chrdev : %d\n",status);
        goto erreurRegister;
    }

    cls = class_create(THIS_MODULE, "Test module\n");
    if(IS_ERR(cls))
    {
        status = PTR_ERR(cls);
        printk(KERN_ERR"Erreur class_create : %d\n",status);
        goto erreurClass;
    }
    
    devt = MKDEV(major,0);
    dev = device_create(cls,NULL,devt,NULL,"testDevice");
    if(IS_ERR(dev))
    {
        status = PTR_ERR(dev);
        printk(KERN_ERR"Erreur device_create : %d\n",status);
        goto erreurDevCreate;
    }
    
    return 0;

erreurDevCreate:
    class_destroy(cls);
erreurClass:
    unregister_chrdev(major,"charDevice");
erreurRegister:
    return status;

}

/*
*   fonctionExit : On libère la mémoire lorsque le module est déchargé.
*/
static void __exit fonctionExit(void)
{
    device_destroy(cls,devt);
    class_destroy(cls);
    unregister_chrdev(major,"FANTOME_DEV");
}

module_init(fonctionInit);
module_exit(fonctionExit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Clement Dugue\nJerome Gauzins\nArthur Canal");
MODULE_DESCRIPTION("Utilisation fichiers virtuels");
