#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/init.h>

#define DEVICE_NAME "char_dev"
#define CLASS_NAME "char_dev_class"
#define BUFFER_SIZE 1024

static int major;
static char device_buffer[BUFFER_SIZE];
static struct cdev *char_dev_cdev;
static struct class *char_dev_class;

static int char_dev_open(struct inode *inode, struct file *filp) {
    pr_info("Char Dev - Opened\n");
    return 0;
}

static int char_dev_release(struct inode *inode, struct file *filp) {
    pr_info("Char Dev - Released\n");
    return 0;
}

static ssize_t char_dev_read(struct file *filp, const char __user *user_buffer, size_t count, loff_t *f_pos) {
    int bytes_read = count > BUFFER_SIZE ? BUFFER_SIZE : count;

    if (copy_to_user(user_buffer, device_buffer, bytes_read)) {
        return -EFAULT;
    }

    pr_info("Char Dev - Read %d bytes\n", bytes_read);
    return bytes_read;
}

static ssize_t char_dev_write(struct file *filp, const char __user *user_buffer, size_t count, loff_t *f_pos) {
    int bytes_write = count > BUFFER_SIZE ? BUFFER_SIZE : count;

    if (copy_from_user(device_buffer, user_buffer, bytes_write)) {
        return -EFAULT;
    }
    
    pr_info("Char Dev - Wrote %d bytes\n", bytes_write);
    return bytes_write;
}

static struct file_operations char_dev_fops = {
    .owner = THIS_MODULE,
    .open = char_dev_open,
    .release = char_dev_release,
    .read = char_dev_read,
    .write = char_dev_write,
};

static int __init char_dev_init(void) {
    static dev_t dev;

    // allocate a device number
    if (alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME) < 0) {
        pr_err("Failed to allocate device number\n");
        return -1;
    }

    major = MAJOR(dev);
    pr_info("Char Dev - Registered with major number %d\n", major);

    // initialize and add cdev
    cdev_init(char_dev_cdev, &char_dev_fops);
    char_dev_cdev->owner = THIS_MODULE;
    if (cdev_add(char_dev_cdev, dev, 1) < 0) {
        pr_err("Failed to add cdev\n");
        goto fail_cdev_add;
    }

    // create a class
    char_dev_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(char_dev_class)) {
        pr_err("Failed to create class\n");
        goto fail_class_create;
    }

    // create a device
    if (IS_ERR(device_create(char_dev_class, NULL, dev, NULL, DEVICE_NAME))) {
        pr_err("Failed to create device\n");
        goto fail_device_create;
    }

    return 0;

fail_device_create:
    class_destroy(char_dev_class);
fail_class_create:
    cdev_del(char_dev_cdev);
fail_cdev_add:
    unregister_chrdev_region(dev, 1);
    return -1;
}

static void __exit char_dev_exit(void) {
    dev_t dev = MKDEV(major, 0);

    device_destroy(char_dev_class, dev);
    class_destroy(char_dev_class);
    cdev_del(char_dev_cdev);
    unregister_chrdev_region(dev, 1);
    pr_info("Char Dev - Cleanup complete\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SB");
MODULE_DESCRIPTION("A simple character device driver");
MODULE_VERSION("0.1");