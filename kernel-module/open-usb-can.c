/*
 * CAN driver for esd CAN-USB/2
 *
 * Copyright (C) 2010 Matthias Fuchs <matthias.fuchs@esd.eu>, esd gmbh
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include <linux/init.h>
#include <linux/signal.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/usb.h>

#include <linux/can.h>
#include <linux/can/dev.h>
#include <linux/can/error.h>

MODULE_AUTHOR("Fabio Baltieri <fabio.baltieri@gmail.com>");
MODULE_DESCRIPTION("CAN driver for Open USB-CAN interfaces");
MODULE_LICENSE("GPL v2");

/* Define these values to match your devices */
#define OPEN_USB_CAN_VENDOR_ID	0x03eb
#define OPEN_USB_CAN_PRODUCT_ID	0xcab5

#define OPEN_USB_CAN_CAN_CLOCK	16000000

#define RX_BUFFER_SIZE		64
#define MAX_RX_URBS		4
#define MAX_TX_URBS		16 /* must be power of 2 */

static struct usb_device_id open_usb_can_table[] = {
	{USB_DEVICE(OPEN_USB_CAN_VENDOR_ID, OPEN_USB_CAN_PRODUCT_ID)},
	{}
};
MODULE_DEVICE_TABLE(usb, open_usb_can_table);

struct usb_tx_urb_context {
	struct open_usb_can *dev;
	u32 echo_index;
	int dlc;
};

struct open_usb_can {
	struct can_priv can; /* must be the first member */
	int open_time;

	struct usb_device *udev;
	struct net_device *netdev;

	atomic_t active_tx_jobs;
	struct usb_anchor tx_submitted;
	struct usb_tx_urb_context tx_contexts[MAX_TX_URBS];

	struct usb_anchor rx_submitted;
};

static void open_usb_can_read_bulk_callback(struct urb *urb)
{
	struct open_usb_can *dev = urb->context;
        struct net_device *netdev = dev->netdev;
	int retval;
	int i;

        struct can_frame *cf;
        struct can_frame *msg;
        struct sk_buff *skb;
        struct net_device_stats *stats = &dev->netdev->stats;

        if (!netif_device_present(netdev))
                return;

	switch (urb->status) {
	case 0: /* success */
		break;

	case -ENOENT:
	case -ESHUTDOWN:
		return;

	default:
		dev_info(dev->udev->dev.parent,
			 "Rx URB aborted (%d)\n", urb->status);
		goto resubmit_urb;
	}

	/* receive one CAN frame */
	/* TODO: multiple message transfers */

        skb = alloc_can_skb(dev->netdev, &cf);
        if (skb == NULL) {
		dev_err(dev->udev->dev.parent,
			"alloc_can_skb failed\n");
                goto resubmit_urb;
	}

	msg = (struct can_frame *)urb->transfer_buffer;

        cf->can_id = le32_to_cpu(msg->can_id);
        cf->can_dlc = msg->can_dlc;
	for (i = 0; i < cf->can_dlc; i++)
		cf->data[i] = msg->data[i];

        netif_rx(skb);

        stats->rx_packets++;
        stats->rx_bytes += cf->can_dlc;

resubmit_urb:
	usb_fill_bulk_urb(urb, dev->udev, usb_rcvbulkpipe(dev->udev, 2),
			  urb->transfer_buffer, RX_BUFFER_SIZE,
			  open_usb_can_read_bulk_callback, dev);

	retval = usb_submit_urb(urb, GFP_ATOMIC);
	if (retval == -ENODEV) {
		netif_device_detach(dev->netdev);
	} else if (retval) {
		dev_err(dev->udev->dev.parent,
			"failed resubmitting read bulk urb: %d\n", retval);
	}
}

/*
 * callback for bulk IN urb
 */
static void open_usb_can_write_bulk_callback(struct urb *urb)
{
	struct usb_tx_urb_context *context = urb->context;
	struct open_usb_can *dev;
	struct net_device *netdev;

	WARN_ON(!context);

	dev = context->dev;
	netdev = dev->netdev;

	/* free up our allocated buffer */
	usb_free_coherent(urb->dev, urb->transfer_buffer_length,
			  urb->transfer_buffer, urb->transfer_dma);

	atomic_dec(&dev->active_tx_jobs);

	if (!netif_device_present(netdev))
		return;

	if (urb->status)
		dev_info(netdev->dev.parent, "Tx URB aborted (%d)\n",
			 urb->status);

	netdev->trans_start = jiffies;

	/* transmission complete interrupt */
        netdev->stats.tx_packets++;
        netdev->stats.tx_bytes += context->dlc;

        can_get_echo_skb(netdev, context->echo_index);

        /* Release context */
        context->echo_index = MAX_TX_URBS;

        if (netif_queue_stopped(netdev))
		netif_wake_queue(netdev);
}

static ssize_t show_version(struct device *d,
			    struct device_attribute *attr, char *buf)
{
	/* struct usb_interface *intf = to_usb_interface(d); */
	/* struct open_usb_can *dev = usb_get_intfdata(intf); */

	/* TODO: control in version */

	return sprintf(buf, "%d.%d.%d\n",
		       0xf,
		       0xf,
		       0xff);
}
static DEVICE_ATTR(version, S_IRUGO, show_version, NULL);

/* TODO: control function */

static int open_usb_can_setup_rx_urbs(struct open_usb_can *dev)
{
	int i, err = 0;

	for (i = 0; i < MAX_RX_URBS; i++) {
		struct urb *urb = NULL;
		u8 *buf = NULL;

		/* create a URB, and a buffer for it */
		urb = usb_alloc_urb(0, GFP_KERNEL);
		if (!urb) {
			dev_warn(dev->udev->dev.parent,
				 "No memory left for URBs\n");
			err = -ENOMEM;
			break;
		}

		buf = usb_alloc_coherent(dev->udev, RX_BUFFER_SIZE, GFP_KERNEL,
					 &urb->transfer_dma);
		if (!buf) {
			dev_warn(dev->udev->dev.parent,
				 "No memory left for USB buffer\n");
			err = -ENOMEM;
			goto freeurb;
		}

		usb_fill_bulk_urb(urb, dev->udev, usb_rcvbulkpipe(dev->udev, 2),
				  buf, RX_BUFFER_SIZE,
				  open_usb_can_read_bulk_callback, dev);
		urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
		usb_anchor_urb(urb, &dev->rx_submitted);

		err = usb_submit_urb(urb, GFP_KERNEL);
		if (err) {
			usb_unanchor_urb(urb);
			usb_free_coherent(dev->udev, RX_BUFFER_SIZE, buf,
					  urb->transfer_dma);
		}

freeurb:
		/* Drop reference, USB core will take care of freeing it */
		usb_free_urb(urb);
		if (err)
			break;
	}

	/* Did we submit any URBs */
	if (i == 0) {
		dev_err(dev->udev->dev.parent, "couldn't setup read URBs\n");
		return err;
	}

	/* Warn if we've couldn't transmit all the URBs */
	if (i < MAX_RX_URBS) {
		dev_warn(dev->udev->dev.parent,
			 "rx performance may be slow\n");
	}

	return 0;
}

/*
 * Start interface
 */
static int open_usb_can_start(struct open_usb_can *dev)
{
	struct net_device *netdev = dev->netdev;
	int err;

	err = open_usb_can_setup_rx_urbs(dev);
	if (err)
		goto failed;

	/* TODO: usb start */

	dev->can.state = CAN_STATE_ERROR_ACTIVE;

	return 0;

failed:
	if (err == -ENODEV)
		netif_device_detach(netdev);

	dev_err(netdev->dev.parent, "couldn't start device: %d\n", err);

	return err;
}

static void unlink_all_urbs(struct open_usb_can *dev)
{
	int i;

	usb_kill_anchored_urbs(&dev->rx_submitted);

	usb_kill_anchored_urbs(&dev->tx_submitted);
	atomic_set(&dev->active_tx_jobs, 0);

	for (i = 0; i < MAX_TX_URBS; i++)
		dev->tx_contexts[i].echo_index = MAX_TX_URBS;
}

static int open_usb_can_open(struct net_device *netdev)
{
	struct open_usb_can *dev = netdev_priv(netdev);
	int err;

	/* common open */
	err = open_candev(netdev);
	if (err)
		return err;

	/* finally start device */
	err = open_usb_can_start(dev);
	if (err) {
                if (err == -ENODEV)
                        netif_device_detach(dev->netdev);

		dev_warn(netdev->dev.parent,
			 "couldn't start device: %d\n", err);

		close_candev(netdev);

		return err;
	}

	dev->open_time = jiffies;

	netif_start_queue(netdev);

	return 0;
}

static netdev_tx_t open_usb_can_start_xmit(struct sk_buff *skb,
				      struct net_device *netdev)
{
	struct open_usb_can *dev = netdev_priv(netdev);
	struct usb_tx_urb_context *context = NULL;
	struct net_device_stats *stats = &netdev->stats;
	struct can_frame *cf = (struct can_frame *)skb->data;
	struct can_frame *msg;
	struct urb *urb;
	u8 *buf;
	int i, err;
	int ret = NETDEV_TX_OK;
	size_t size = sizeof(struct can_frame);

	if (can_dropped_invalid_skb(netdev, skb))
		return NETDEV_TX_OK;

	/* create a URB, and a buffer for it, and copy the data to the URB */
	urb = usb_alloc_urb(0, GFP_ATOMIC);
	if (!urb) {
		dev_err(netdev->dev.parent, "No memory left for URBs\n");
		goto nourbmem;
	}

	buf = usb_alloc_coherent(dev->udev, size, GFP_ATOMIC,
				 &urb->transfer_dma);
	if (!buf) {
		dev_err(netdev->dev.parent, "No memory left for USB buffer\n");
		goto nobufmem;
	}

	msg = (struct can_frame *)buf;

	msg->can_id = cpu_to_le32(cf->can_id);
	msg->can_dlc = cf->can_dlc;
	for (i = 0; i < cf->can_dlc; i++)
		msg->data[i] = cf->data[i];

	for (i = 0; i < MAX_TX_URBS; i++) {
		if (dev->tx_contexts[i].echo_index == MAX_TX_URBS) {
			context = &dev->tx_contexts[i];
			break;
		}
	}

	/*
	 * This may never happen.
	 */
	if (!context) {
		dev_warn(netdev->dev.parent, "couldn't find free context\n");
		ret = NETDEV_TX_BUSY;
		goto releasebuf;
	}

	context->dev = dev;
	context->echo_index = i;
	context->dlc = cf->can_dlc;

	usb_fill_bulk_urb(urb, dev->udev, usb_sndbulkpipe(dev->udev, 1), buf,
			  size, open_usb_can_write_bulk_callback, context);
	urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
	usb_anchor_urb(urb, &dev->tx_submitted);

	can_put_echo_skb(skb, netdev, context->echo_index);

	atomic_inc(&dev->active_tx_jobs);

	/* Slow down tx path */
	if (atomic_read(&dev->active_tx_jobs) >= MAX_TX_URBS)
		netif_stop_queue(netdev);

	err = usb_submit_urb(urb, GFP_ATOMIC);
	if (unlikely(err)) {
		can_free_echo_skb(netdev, context->echo_index);

		atomic_dec(&dev->active_tx_jobs);
		usb_unanchor_urb(urb);

		stats->tx_dropped++;

		if (err == -ENODEV)
			netif_device_detach(netdev);
		else
			dev_warn(netdev->dev.parent, "failed tx_urb %d\n", err);

		goto releasebuf;
	}

	netdev->trans_start = jiffies;

	/*
	 * Release our reference to this URB, the USB core will eventually free
	 * it entirely.
	 */
	usb_free_urb(urb);

	return NETDEV_TX_OK;

releasebuf:
	usb_free_coherent(dev->udev, size, buf, urb->transfer_dma);

nobufmem:
	usb_free_urb(urb);

nourbmem:
	stats->tx_dropped++;
	dev_kfree_skb(skb);

	return NETDEV_TX_OK;
}

static int open_usb_can_close(struct net_device *netdev)
{
	struct open_usb_can *dev = netdev_priv(netdev);

	unlink_all_urbs(dev);

	netif_stop_queue(netdev);

	/* TODO: set CAN controller to reset mode */

	close_candev(netdev);

	dev->open_time = 0;

	return 0;
}

static const struct net_device_ops open_usb_can_netdev_ops = {
	.ndo_open = open_usb_can_open,
	.ndo_stop = open_usb_can_close,
	.ndo_start_xmit = open_usb_can_start_xmit,
};

static struct can_bittiming_const open_usb_can_bittiming_const = {
	.name = "open_usb_can",
        .tseg1_min = 3,
        .tseg1_max = 16,
        .tseg2_min = 2,
        .tseg2_max = 8,
        .sjw_max = 4,
        .brp_min = 1,
        .brp_max = 64,
        .brp_inc = 1,
};

static int open_usb_can_set_bittiming(struct net_device *netdev)
{
	struct open_usb_can *dev = netdev_priv(netdev);
	struct can_bittiming *bt = &dev->can.bittiming;

	dev_info(netdev->dev.parent, "setting timing for %d bps = %d %d %d %d %d %d\n",
		 bt->bitrate,
		 bt->sjw,
		 bt->brp,
		 dev->can.ctrlmode,
		 bt->phase_seg1,
		 bt->phase_seg2,
		 bt->prop_seg);

	/* TODO: control writes for each parameter*/

	/* return open_usb_can_send_msg(dev->usb2, &msg); */

	return 0;
}

/* static int open_usb_can_get_berr_counter(const struct net_device *netdev, */
/* 				     struct can_berr_counter *bec) */
/* { */
/* 	struct open_usb_can *dev = netdev_priv(netdev); */

/* 	bec->txerr = dev->bec.txerr; */
/* 	bec->rxerr = dev->bec.rxerr; */

/* 	return 0; */
/* } */

static int open_usb_can_set_mode(struct net_device *netdev, enum can_mode mode)
{
	struct open_usb_can *dev = netdev_priv(netdev);

	if (!dev->open_time)
		return -EINVAL;

	switch (mode) {
	case CAN_MODE_START:
		netif_wake_queue(netdev);
		break;

	default:
		return -EOPNOTSUPP;
	}

	return 0;
}

/*
 * probe function for new Open CAN-USB devices
 */
static int open_usb_can_probe(struct usb_interface *intf,
			 const struct usb_device_id *id)
{
	struct net_device *netdev;
	struct open_usb_can *dev;
	int i, err;

	netdev = alloc_candev(sizeof(struct open_usb_can), MAX_TX_URBS);
	if (!netdev) {
		dev_err(&intf->dev, "couldn't alloc candev\n");
		err = -ENOMEM;
		goto done;
	}

	dev = netdev_priv(netdev);

	dev->udev = interface_to_usbdev(intf);
	dev->netdev = netdev;

	dev->can.state = CAN_STATE_STOPPED;
	dev->can.clock.freq = OPEN_USB_CAN_CAN_CLOCK;
	dev->can.bittiming_const = &open_usb_can_bittiming_const;
	dev->can.do_set_bittiming = open_usb_can_set_bittiming;
	dev->can.do_set_mode = open_usb_can_set_mode;
	/* dev->can.do_get_berr_counter = open_usb_can_get_berr_counter; */
	dev->can.ctrlmode_supported = CAN_CTRLMODE_3_SAMPLES;

	netdev->netdev_ops = &open_usb_can_netdev_ops;

	netdev->flags |= IFF_ECHO; /* we support local echo */

	init_usb_anchor(&dev->rx_submitted);

	init_usb_anchor(&dev->tx_submitted);
	atomic_set(&dev->active_tx_jobs, 0);

	for (i = 0; i < MAX_TX_URBS; i++)
		dev->tx_contexts[i].echo_index = MAX_TX_URBS;

	usb_set_intfdata(intf, dev);

	SET_NETDEV_DEV(netdev, &intf->dev);

	if (device_create_file(&intf->dev, &dev_attr_version))
		dev_err(&intf->dev,
			"Couldn't create device file for firmware\n");

	err = register_candev(netdev);
	if (err) {
		dev_err(&intf->dev,
			"couldn't register CAN device: %d\n", err);
		free_candev(netdev);
		err = -ENOMEM;
		goto done;
	}

	dev_info(&netdev->dev,
		"Open USB-CAN device probed\n");

	return 0;

done:
	return err;
}

/*
 * called by the usb core when the device is removed from the system
 */
static void open_usb_can_disconnect(struct usb_interface *intf)
{
	struct open_usb_can *dev = usb_get_intfdata(intf);

	device_remove_file(&intf->dev, &dev_attr_version);

	usb_set_intfdata(intf, NULL);

	if (dev) {
		unregister_netdev(dev->netdev);
		free_candev(dev->netdev);

		unlink_all_urbs(dev);
	}
}

/* usb specific object needed to register this driver with the usb subsystem */
static struct usb_driver open_usb_can_driver = {
	.name = "open_usb_can",
	.probe = open_usb_can_probe,
	.disconnect = open_usb_can_disconnect,
	.id_table = open_usb_can_table,
};

static int __init open_usb_can_init(void)
{
	int err;

        printk(KERN_INFO "Open USB-CAN kernel driver loaded\n");

	/* register this driver with the USB subsystem */
	err = usb_register(&open_usb_can_driver);

	if (err) {
		err("usb_register failed. Error number %d\n", err);
		return err;
	}

	return 0;
}
module_init(open_usb_can_init);

static void __exit open_usb_can_exit(void)
{
	/* deregister this driver with the USB subsystem */
	usb_deregister(&open_usb_can_driver);
}
module_exit(open_usb_can_exit);
