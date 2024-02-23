#include <linux/module.h>
#include <linux/init.h>
#include <linux/hid.h>
#include <linux/input.h>
#define VENDOR 0x0930
#define PRODUCT 0x0807
MODULE_LICENSE("GPL");
MODULE_AUTHOR("cylian91");
MODULE_DESCRIPTION("A simple driver for toshiba Z10T-A");

static const struct hid_device_id tba_ids[] = {
  {HID_USB_DEVICE(VENDOR,PRODUCT)},
  {}
};
MODULE_DEVICE_TABLE(hid,tba_ids);

static int tba_probe(struct hid_device *device, const struct hid_device_id *device_id){
  printk("probed");
  return -1;
}
static struct hid_driver tba_hid = {
  .name = "Z10T-A driver",
  .probe = tba_probe,
  .id_table = tba_ids
};




static int __init startDriver(void){
  printk("hello Kernel\n");
  if (hid_register_driver(&tba_hid)){
    printk("didn't work");
  }
  return 0;
}
static void __exit stopDriver(void){
  hid_unregister_driver(&tba_hid);
  printk("bye kernel\n");
}

module_init(startDriver);
module_exit(stopDriver);
