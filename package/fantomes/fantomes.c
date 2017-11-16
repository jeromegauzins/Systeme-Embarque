#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/fs.h>

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
		b[i] = i;
	printk(KERN_INFO"Read!!!!\n");
	copy_to_user(data, buf, size);
	kfree(buf);
	return size;
}

static ssize_t d_write(struct file *fp, const char __user *data, size_t size, loff_t *l)
{
    char *plop = kmalloc(size + 1, GFP_KERNEL);
	copy_from_user(plop, buf, size);
	plop[size] = 0;

	printk(KERN_INFO "J'ai reçu ça: %s\n", plop);
	kfree(plop);
	return size;
}

static int __init fonctionInit(void)
{
    int status;
    major = register_chrdev(0,"POULET",&fops);//Fichier dans /dev/devices ?
    if(major < 0)
    {
        printk(KERN_INFO"Erreur register_chrdev");
        status = major;
        goto erreurRegister;
    }

    c = class_create(THIS_MODULE, "Test module");
    if(IS_ERR(c))
    {
        printk(KERN_INFO"Erreur register_chrdev");
        status = PTR_ERR(c);
        goto erreurClass;
    }
    
    devt = MKDEV(major,0);
    d = device_create(c,NULL,devt,NULL,"testDevice");
    
    status = IS_ERR(d) ? PTR_ERR(d) : 0;

    if(status != 0)
    {
        printk(KERN_INFO"Erreur register_chrdev");

        goto erreurDevice;
    }

    return 0;

erreurDevice:
    class_destroy(c);
erreurClass:
    unregister_chrdev(major,"POULET");
erreurRegister:
    return status;

}

static void __exit fonctionExit(void)
{
    device_destroy(c,devt);
    class_destroy(c);
    unregister_chrdev(major,"POULET");

}

module_init(fonctionInit);
module_exit(fonctionExit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Clement Dugue\nJerome Gauzins\nArthur Canal");
MODULE_DESCRIPTION("Utilisation fichiers virtuels");
