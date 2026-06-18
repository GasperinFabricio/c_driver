#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio/consumer.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>       
#include <linux/device.h>

// Variáveis configuráveis via parâmetro (com valores padrão)
static int gpio_pin = 21;
static int gpio_offset = 512;

// Macros que expõem as variáveis para o comando insmod
module_param(gpio_pin, int, 0644);
MODULE_PARM_DESC(gpio_pin, "Número do pino físico do GPIO (Padrão: 21)");

module_param(gpio_offset, int, 0644);
MODULE_PARM_DESC(gpio_offset, "Offset de virtualização do gpiochip (Padrão: 512)");

static struct gpio_desc *led;

// Variáveis do Driver CDEV
static dev_t dev_nr;
static struct cdev my_cdev;
static struct class *my_class;

// Implementação de fops
static ssize_t my_read(struct file *f, char __user *u, size_t l, loff_t *o)
{
    return 0;
}

static ssize_t my_write(struct file *f, const char __user *buf, size_t count, loff_t *f_pos)
{
    char input_char;

    if (count < 1) return -EINVAL;

    if (copy_from_user(&input_char, buf, 1)) {
        return -EFAULT;
    }

    if (input_char == '1') {
        gpiod_set_value(led, 1);
        pr_info("gpio_tcc - LED LIGADO\n");
    }
    else if (input_char == '0') {
        gpiod_set_value(led, 0);
        pr_info("gpio_tcc - LED DESLIGADO\n");
    }

    return count;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = my_read,
    .write = my_write
};

// Inicialização
static int __init my_init(void)
{
    int status;
    int pino_virtual = gpio_pin + gpio_offset;

    // Obtém o descritor usando a soma dinâmica
    led = gpio_to_desc(pino_virtual);
    if (!led) {
        printk("gpioctrl - Erro ao captar pino virtual %d (Físico: %d)\n", pino_virtual, gpio_pin);
        return -ENODEV;
    }

    status = gpiod_direction_output(led, 0);
    if (status) {
        printk("gpioctrl - Erro ao setar pino %d para output\n", gpio_pin);
        return status;
    }

    // Alocar Major/Minor
    status = alloc_chrdev_region(&dev_nr, 0, 1, "gpio_tcc_driver");
    if (status) {
        printk("gpioctrl - Erro alocando chrdev\n");
        return status;
    }

    // Inicializar Cdev
    cdev_init(&my_cdev, &fops);
    my_cdev.owner = THIS_MODULE;

    // Adicionar Cdev
    status = cdev_add(&my_cdev, dev_nr, 1);
    if (status) {
        printk("gpioctrl - Error adicionando cdev\n");
        unregister_chrdev_region(dev_nr, 1);
        return status;
    }

    // Criar Classe e Device (Automático /dev/gpio_tcc_dev)
    my_class = class_create("gpio_tcc_class");
    if (IS_ERR(my_class)) {
        printk("gpioctrl - Error criando class\n");
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev_nr, 1);
        return PTR_ERR(my_class);
    }

    device_create(my_class, NULL, dev_nr, NULL, "gpio_tcc_dev");

    printk("gpioctrl - Driver inicializado no pino %d (Virtual: %d). Escreva 1/0 para /dev/gpio_tcc_dev\n", gpio_pin, pino_virtual);
    return 0;
}

// Saída
static void __exit my_exit(void)
{
    if (led) {
        gpiod_set_value(led, 0);
    }

    device_destroy(my_class, dev_nr);
    class_destroy(my_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_nr, 1);

    printk("gpioctrl - Driver removido\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Fabricio Gasperin");
MODULE_DESCRIPTION("TCC Driver");