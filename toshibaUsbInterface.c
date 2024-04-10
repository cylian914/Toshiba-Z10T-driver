#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/usb.h>
#include <linux/usb/input.h>
#include <linux/hid.h>
#define VENDOR 0x0930
#define PRODUCT 0x0807
MODULE_LICENSE("GPL");
MODULE_AUTHOR("cylian91");
MODULE_DESCRIPTION("A simple driver for toshiba Z10T-A");
#define USBMOUSE 2
static const struct usb_device_id tba_ids[] = {
	{USB_DEVICE(VENDOR,PRODUCT)},
	{}
};
MODULE_DEVICE_TABLE(usb,tba_ids);
/* reverse note:
 * unchar 06 : type ?
 * unchar click: left 1 right 2
 * char relat pos x
 * char relat pos y
 * 2 null byte ?
 */
struct tba_proto {
	unsigned char type;
	unsigned char click;
	char pos_x;
	char pos_y;
	char unknown;
};
struct mouse {
	char phys[128];
	char name[64];

	struct urb* irq;
	struct usb_device *udev;
	struct input_dev *input;
	struct tba_proto *data;
	dma_addr_t data_dma;
};

static void tba_mouse_irq(struct urb *urb){
	struct mouse* mouse = urb->context;
	struct tba_proto *data = mouse->data;
	struct input_dev *dev = mouse->input;
	printk("irq: %i\n",urb->status);
	switch (urb->status) {
		case 0:			/* success */
			break;
		case -ECONNRESET:	/* unlink */
		case -ENOENT:
		case -ESHUTDOWN:
			return;
			/* -EPIPE:  should clear the halt */
		default:		/* error */
			goto resubmit;
	}
	printk("data: %x,%x,%x,%x,%x\n",data->type,data->click,data->pos_x,data->pos_y,data->unknown);
	input_report_key(dev, BTN_LEFT, data->click & 0x01);
	input_report_key(dev, BTN_RIGHT, data->click & 0x02);
	input_report_rel(dev, REL_X, data->pos_x);
	input_report_rel(dev, REL_Y, data->pos_y);

	input_sync(dev);
resubmit:
	int status = usb_submit_urb(urb,GFP_KERNEL);
}

static int tba_input_open(struct input_dev *idev){
	struct mouse *mouse = input_get_drvdata(idev);
	printk("open\n");
	int status = usb_submit_urb(mouse->irq,GFP_KERNEL);
	printk("open, urb:%i\n",status);
	//printk("%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i",-ENOMEM, -ENODEV,-ENOENT,-ENXIO,-EINVAL,-EXDEV,-EFBIG,-EPIPE,-EMSGSIZE,-EBADR,-ENOSPC,-ESHUTDOWN,EPERM,-EHOSTUNREACH,-ENOEXEC);
	if (status)
		return -EIO;
	printk("open,not failed ?\n");
	return 0; 
}
static void tba_input_close(struct input_dev *idev){
	struct mouse *mouse = input_get_drvdata(idev);
	printk("close\n");
	usb_kill_urb(mouse->irq);
	return;
}
static int tba_probe(struct usb_interface *intf, const struct usb_device_id *device_id){
	struct usb_device *udev;
	udev = interface_to_usbdev(intf);
	printk("probed: %s, %i\n", udev->product, intf->cur_altsetting->desc.bInterfaceProtocol);
	if (intf->cur_altsetting->desc.bInterfaceProtocol != USBMOUSE)
	  return -1;

	printk("Pad detected\n"); 

	struct input_dev *indev;
	struct mouse *mouse = kzalloc(sizeof(mouse),GFP_KERNEL);
	mouse->irq = usb_alloc_urb(0, GFP_KERNEL);
	unsigned int pipe = usb_rcvintpipe(udev, intf->cur_altsetting->endpoint[0].desc.bEndpointAddress);

	mouse->data = usb_alloc_coherent(udev,sizeof(struct tba_proto),GFP_KERNEL,&mouse->data_dma);
	if (!mouse->data) goto fail1;

	indev = input_allocate_device();
	if (!indev) goto fail2;
	printk("input device allocated\n");
	usb_make_path(udev,mouse->phys,sizeof(mouse->phys));
	printk("path: %s\n",mouse->phys);

	printk("aaa");
	snprintf(mouse->phys,sizeof(mouse->phys),"%s/input0",mouse->phys);
	printk("bbbb");
	printk("path: %s\n",mouse->phys);
	mouse->input = indev;
	snprintf(mouse->name,63,"Toshiba usb driver");
	printk("input device: %s\n",mouse->name);
	usb_to_input_id(udev,&indev->id);
	indev->dev.parent=&intf->dev;
	printk("ut device: %s\n",mouse->name);
	indev->name = mouse->name;
	indev->phys = mouse->phys;

	indev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REL);
	indev->keybit[BIT_WORD(BTN_MOUSE)] = BIT_MASK(BTN_LEFT) | BIT_MASK(BTN_RIGHT);
	indev->relbit[0] = BIT_MASK(REL_X) | BIT_MASK(REL_Y);

	indev->open = tba_input_open;
	indev->close = tba_input_close;
	usb_fill_int_urb(mouse->irq,udev,pipe,mouse->data,sizeof(mouse->data),tba_mouse_irq,mouse,intf->cur_altsetting->endpoint[0].desc.bInterval);
	mouse->irq->transfer_dma = mouse->data_dma;
	mouse->irq->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
	input_register_device(mouse->input);
	usb_set_intfdata(intf, mouse);
	input_set_drvdata(indev, mouse);
	return 0;
fail3:
	input_free_device(mouse->input);
fail2:
	usb_free_coherent(udev,sizeof(struct tba_proto),mouse->data,mouse->data_dma);
fail1:
	usb_free_urb(mouse->irq);
fail:
	kfree(mouse);
	return -1;
}

static void tba_remove(struct usb_interface *intf){
	struct mouse *mouse = usb_get_intfdata(intf);
	if (mouse){	
				//printk("removed: %s, %i\n", mouse->udev->product, intf->cur_altsetting->desc.bInterfaceProtocol);
		input_unregister_device(mouse->input);
		usb_free_urb(mouse->irq);
		usb_free_coherent(interface_to_usbdev(intf), sizeof(mouse->data), mouse->data, mouse->data_dma);
		kfree(mouse);
	} 
}

static struct usb_driver tba_usb = {
	.name = "Z10T-A driver",
	.probe = tba_probe,
	.disconnect = tba_remove,
	.id_table = tba_ids
};

module_usb_driver(tba_usb);
