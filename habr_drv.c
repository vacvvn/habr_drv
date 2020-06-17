/*
статья на хабре: 
https://habr.com/ru/post/106702/

........................................
root@joker:/tmp/habr_drv# make
make -C /lib/modules/4.15.0-106-generic/build M=/home/vacvvn/software/examples/kernel_example/habr_drv modules
make[1]: Entering directory '/usr/src/linux-headers-4.15.0-106-generic'
  CC [M]  /home/vacvvn/software/examples/kernel_example/habr_drv/habr_drv.o
  Building modules, stage 2.
  MODPOST 1 modules
  LD [M]  /home/vacvvn/software/examples/kernel_example/habr_drv/habr_drv.ko
make[1]: Leaving directory '/usr/src/linux-headers-4.15.0-106-generic'

Теперь посмотрим информацию о только что скомпилированном модуле:

root@joker:/tmp/habr_drv# modinfo habr_drv.ko
filename: habr_drv.ko
description: My nice module
author: Alex Petrov <druid@joker.botik.ru>
license: GPL
depends:
vermagic: 2.6.26-2-openvz-amd64 SMP mod_unload modversions

Ну и наконец установим модуль в ядро:

root@joker:/tmp/habr_drv# insmod habr_drv.ko

Посмотрим есть ли наш модуль с списке:

root@joker:/tmp/habr_drv# lsmod | grep habr_drv

habr_drv 6920 0


И что попало в логи:

root@joker:/tmp/habr_drv# dmesg | tail

[829528.598922] Test module is loaded!
[829528.598926] Please, create a dev file with 'mknod /dev/habr_drv c 249 0'.


Наш модуль подсказываем нам что нужно сделать.

Последуем его совету:

root@joker:/tmp/habr_drv# mknod /dev/habr_drv c 249 0

Ну и наконец проверим работает ли наш модуль:

root@joker:/tmp/habr_drv# cat /dev/habr_drv

habr_drv

Наш модуль не поддерживает приём данных со стороны пользователя:

root@joker:/tmp/habr_drv# echo 1 > /dev/habr_drv

bash: echo: ошибка записи: Недопустимый аргумент

Посмотрим что что скажет модуль на наши действия:

root@joker:/tmp/habr_drv# dmesg | tail

[829528.598922] Test module is loaded!
[829528.598926] Please, create a dev file with 'mknod /dev/habr_drv c 249 0'.
[829747.462715] Sorry, this operation isn't supported.


Удалим его:

root@joker:/tmp/habr_drv# rmmod habr_drv

И посмотрим что он нам скажет на прощание:

root@joker:/tmp/habr_drv# dmesg | tail

[829528.598922] Test module is loaded!
[829528.598926] Please, create a dev file with 'mknod /dev/habr_drv c 249 0'.
[829747.462715] Sorry, this operation isn't supported.
[829893.681197] Test module is unloaded!


Удалим файл устройства, что бы он нас не смущал:

root@joker:/tmp/habr_drv# rm /dev/habr_drv
*/
#include <linux/kernel.h> /* Для printk() и т.д. */
#include <linux/module.h> /* Эта частичка древней магии, которая оживляет модули */
#include <linux/init.h>   /* Определения макросов */
#include <linux/fs.h>
/*
Hi! When building the enhanced example with character device, I encountered a build error for kernel 4.15.0–52-generic on 64-bit Mint/Ubuntu:
```
./arch/x86/include/asm/uaccess.h: In function ‘set_fs’: ./arch/x86/include/asm/uaccess.h:32:9: error: dereferencing pointer to incomplete type ‘struct task_struct’ current->thread.addr_limit = fs;
```
The answer to this problem is to include linux/uaccess.h header instead of asm/uaccess.h
*/
// #include <asm/uaccess.h> /* put_user */
#include <linux/uaccess.h> /* put_user */

// Ниже мы задаём информацию о модуле, которую можно будет увидеть с помощью Modinfo
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alex Petrov <petroff.alex@gmail.com>");
MODULE_DESCRIPTION("My nice module");
MODULE_SUPPORTED_DEVICE("habr_drv"); /* /dev/habr_drvdevice */

#define SUCCESS 0
#define DEVICE_NAME "habr_drv" /* Имя нашего устройства */

// Поддерживаемые нашим устройством операции
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

// Глобальные переменные, объявлены как static, воизбежание конфликтов имен.
static int major_number;        /* Старший номер устройства нашего драйвера */
static int is_device_open = 0;  /* Используется ли девайс ? */
static char text[5] = "habr_drv\n"; /* Текст, который мы будет отдавать при обращении к нашему устройству */
static char *text_ptr = text;   /* Указатель на текущую позицию в тексте */

// Прописываем обработчики операций на устройством
static struct file_operations fops =
    {
        .read = device_read,
        .write = device_write,
        .open = device_open,
        .release = device_release};

// Функция загрузки модуля. Входная точка. Можем считать что это наш main()
static int __init habr_drv_init(void)
{
    printk(KERN_ALERT "TEST driver loaded!\n");

    // Регистрируем устройсво и получаем старший номер устройства
    major_number = register_chrdev(0, DEVICE_NAME, &fops);

    if (major_number < 0)
    {
        printk("Registering the character device failed with %d\n", major_number);
        return major_number;
    }

    // Сообщаем присвоенный нам старший номер устройства
    printk("Test module is loaded!\n");

    printk("Please, create a dev file with 'mknod /dev/habr_drv c %d 0'.\n", major_number);

    return SUCCESS;
}

// Функция выгрузки модуля
static void __exit habr_drv_exit(void)
{
    // Освобождаем устройство
    unregister_chrdev(major_number, DEVICE_NAME);

    printk(KERN_ALERT "Test module is unloaded!\n");
}

// Указываем наши функции загрузки и выгрузки
module_init(habr_drv_init);
module_exit(habr_drv_exit);

static int device_open(struct inode *inode, struct file *file)
{
    text_ptr = text;

    if (is_device_open)
        return -EBUSY;

    is_device_open++;

    return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file)
{
    is_device_open--;
    return SUCCESS;
}

static ssize_t

device_write(struct file *filp, const char *buff, size_t len, loff_t *off)
{
    printk("Sorry, this operation isn't supported.\n");
    return -EINVAL;
}

static ssize_t device_read(struct file *filp, /* include/linux/fs.h */
                           char *buffer,      /* buffer */
                           size_t length,     /* buffer length */
                           loff_t *offset)
{
    int byte_read = 0;

    if (*text_ptr == 0)
        return 0;
    printk(KERN_INFO "[device read func]\n>");
    while (length && *text_ptr)
    {
        put_user(*(text_ptr++), buffer++);
        // printk(KERN_INFO "%c",*text_ptr++);
        length--;
        byte_read++;
    }

    return byte_read;
}
