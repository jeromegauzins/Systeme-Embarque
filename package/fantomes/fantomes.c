#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/fs.h>

int major;

struct device *dev;
static struct class *cls;
dev_t devt;

//Fonctions de callback
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

static int __init fonctionInit(void);
static void __exit fonctionExit(void);

static int d_open(struct inode *i, struct file *fp)
{
    printk(KERN_INFO"Le fichier a ete ouvert\n");
    return 0;
}

static int d_release(struct inode *i, struct file *fp)
{
    printk(KERN_INFO"Le fichier a ete ferme\n");
    return 0;
}

static ssize_t d_read(struct file *fp, char __user *data, size_t size, loff_t *l)
{
	printk(KERN_INFO"Read!!!!\n");
	return size;
}

static ssize_t d_write(struct file *fp, const char __user *data, size_t size, loff_t *l)
{
    char *msg = kmalloc(size + 1, GFP_KERNEL);
	copy_from_user(msg, buf, size);
	msg[size] = 0;

	printk(KERN_INFO "Message recu : %s\n", msg);
	kfree(msg);
	return size;
}

static int __init fonctionInit(void)
{
    int status;
    //Initialisation du character device
    major = register_chrdev(0,"charDevice",&fops);
    if(major < 0)
    {
        printk(KERN_INFO"Erreur register_chrdev\n");
        status = major;
        goto erreurRegister;
    }

    cls = class_create(THIS_MODULE, "Test module\n");
    if(IS_ERR(cls))
    {
        printk(KERN_INFO"Erreur register_chrdev\n");
        status = PTR_ERR(cls);
        goto erreurClass;
    }
    
    devt = MKDEV(major,0);
    dev = device_create(cls,NULL,devt,NULL,"testDevice\n");
    
    status = IS_ERR(dev) ? PTR_ERR(dev) : 0;

    if(status != 0)
    {
        printk(KERN_INFO"Erreur register_chrdev\n");

        goto erreurDevice;
    }

    return 0;

erreurDevice:
    class_destroy(cls);
erreurClass:
    unregister_chrdev(major,"POULET");
erreurRegister:
    return status;

}

static void __exit fonctionExit(void)
{
    device_destroy(cls,devt);
    class_destroy(cls);
    unregister_chrdev(major,"POULET");

}

module_init(fonctionInit);
module_exit(fonctionExit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Clement Dugue\nJerome Gauzins\nArthur Canal");
MODULE_DESCRIPTION("Utilisation fichiers virtuels");
