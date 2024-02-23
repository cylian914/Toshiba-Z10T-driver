#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb.h>
#define VENDOR 0x0930
#define PRODUCT 0x0807
MODULE_LICENSE("GPL");
MODULE_AUTHOR("cylian91");
MODULE_DESCRIPTION("A simple driver for toshiba usb interface");

static struct usb_device_id tba_ids[] = {
  {USB_DEVICE(VENDOR,PRODUCT) },
  {0}
};
MODULE_DEVICE_TABLE (usb, tba_ids);
void tba_disco(struct usb_interface *interface){
  printk("disconnected");
  return;
}
static struct usb_driver tba_driver = {
  .name = "Toshiba T10-A driver",
  .disconnect = tba_disco,
  .id_table = tba_ids
  
};



static int __init startDriver(void){
  printk("hello Kernel\n");
  usb_register_driver(&tba_driver);
  return 0;
}
static void __exit stopDriver(void){
  usb_deregister(&tba_driver);
  printk("bye kernel\n");
}

module_init(startDriver);
module_exit(stopDriver);
