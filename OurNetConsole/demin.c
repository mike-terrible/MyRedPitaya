#include <linux/module.h>
#include <linux/printk.h>
#include <linux/kernel.h>
#include <linux/string.h>

#include <linux/err.h>

#include <linux/fs.h>

#include <linux/ioport.h>
#include <linux/jiffies.h>

#include <linux/platform_device.h>

#include <linux/irq.h>

#include <linux/uio.h>

#include <linux/phy.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/if_ether.h>
#include <linux/mii.h>

#include <linux/socket.h>
#include <linux/net.h>
#include <linux/in.h>
#include <linux/udp.h>

#include <linux/dma-mapping.h>

#define DEVICE_NAME "demin"
#define DRIVER_AUTHOR "miketerrible@gmail.com"
#define DRIVER_DESC "net device module for testing"

static struct platform_device *pd=NULL;
static struct net_device *demin;

static void *tester=NULL;
static dma_addr_t DM;

/************  net console     ******************/
static int MyConOk=0;
struct socket* MyCon=NULL;

static int net_err=0;

static struct sockaddr_in a;
static unsigned char cbuf[1024];
static struct kvec kkv;
static struct iov_iter io;
static struct msghdr mh;
//static unsigned char svaddr[]={ 192,168,10,101 };
static unsigned char svaddr[]={ 127,0,0,1 };

static char prtbuf[1024];

#define bzero(a,n) memset(a,0,n)

static void out(char* s) {
  int i;
  i=0;
  while(i<1024) {
    if(s[i]==']') { s[i]=0; break; };
    i++;
  };
  if(!MyConOk) { printk(s); return; };
  kkv.iov_base=s;
  kkv.iov_len=1024;
  io.iov_offset=0;
  io.count=1024;
  io.nr_segs=1024;
  io.kvec=&kkv;
  memcpy(&mh.msg_iter,&io,sizeof io);
  //kkv.iov_base=kmalloc(1024,GFP_KERNEL);
  //memcpy(kkv.iov_base,s,1024);
  //bzero(cbuf,1024);
  //cbuf[0]=65; cbuf[1]=66; cbuf[2]=67;
  sock_sendmsg(MyCon,&mh); 
}

static void myConInit(void) {
  bzero(&kkv,sizeof(struct kvec));
  kkv.iov_len=1024;
  bzero(&io,sizeof io);
  //iov_iter_init(&io, WRITE, &kkv, 1, 1024);
  io.type=ITER_KVEC;
  io.count=1;
  io.nr_segs=1024;
  io.kvec=&kkv;
  bzero(&mh,sizeof mh);
  mh.msg_name=&a;
  mh.msg_namelen=sizeof a;
  memcpy(&mh.msg_iter,&io,sizeof io);
  bzero(&a, sizeof a); bzero(cbuf,1024);
  a.sin_family = AF_INET;
  memcpy(&a.sin_addr.s_addr,svaddr,4); 
  a.sin_port = htons(1025);
  net_err=sock_create(AF_INET, SOCK_DGRAM, IPPROTO_UDP, &MyCon);
  if(net_err==0) MyConOk=1;
}

/*****************************/


static int demin_ioctl(struct net_device *dev,struct ifreq *ifr, int cmd) {
  printk(KERN_INFO "demin: do_ioctl cmd=%x]",cmd);
  return 0;
}

static int demin_open (struct net_device *dev)
{ 
  sprintf(prtbuf,"demin: demin_open called (ip link set eth1 up)]");
  out(prtbuf);
  ////dev->state=__LINK_STATE_START|__LINK_STATE_PRESENT;
  //dev->flags |= IFF_UP|IFF_LOWER_UP|IFF_BROADCAST;
  netif_start_queue (dev);
  //netif_carrier_on(demin);
  return 0;
}

static int demin_release (struct net_device *dev)
{ 
  sprintf(prtbuf,"demin: demin_release called (ip link set eth1 down)]");
  out(prtbuf);
  //dev->state=__LINK_STATE_PRESENT;
  netif_stop_queue(dev);
  return 0;
}


static int demin_xmit (struct sk_buff *skb,struct net_device *dev)
{ dev->trans_start=jiffies;
  sprintf(prtbuf,"demin: %lu dummy xmit function called....]",dev->trans_start);
  out(prtbuf);
  //printk (KERN_INFO "demin: protocol %04X\n",eth_type_trans(skb,dev));
  sprintf(prtbuf,"demin: protocol %04X]",htons(skb->protocol));
  out(prtbuf);
  sprintf(prtbuf,"demin: skb len is %u]",skb->len);
  out(prtbuf);
  sprintf(prtbuf,"demin: skb data_len is %u]",skb->data_len);
  out(prtbuf);
  sprintf(prtbuf,"demin: skb truesize is %u]",skb->truesize);
  out(prtbuf);
  //netif_rx(skb);
  //if (skb->stamp.tv_sec == 0) net_timestamp(&skb->stamp);
  dev_kfree_skb(skb);
  return NETDEV_TX_OK;
}

static int demin_set_mac_address(struct net_device *ndev,void *address) {
  //memcpy(ndev->dev_addr,address+2,6);
  return eth_mac_addr(ndev,address);
}

static int demin_init(struct net_device *dev) {
  //dev->flags=IF_OPER_UP|IFF_UP|IFF_BROADCAST;
  dev->state=__LINK_STATE_START|__LINK_STATE_PRESENT;
  //dev->flags = IFF_UP;
  //netif_carrier_off(demin);
  //dev->flags |= IFF_UP|IFF_RUNNING;
  //dev->flags |= IF_OPER_UP|IFF_UP;
  dev->flags |= IFF_NOARP|IFF_UP|IFF_RUNNING;
  dev->operstate |= IF_OPER_UP;
  sprintf(prtbuf,"demin: init]");
  return 0;
}

static void demin_uninit(struct net_device *dev) {
  sprintf(prtbuf,"demin: uninit]"); out(prtbuf);
  dev->state = 0;
  dev->flags = 0;
}

static struct net_device_ops demin_ops= {
  .ndo_init         = demin_init,
  .ndo_uninit       = demin_uninit,
  .ndo_open         = demin_open,
  .ndo_stop         = demin_release,
  .ndo_start_xmit   = demin_xmit,
  .ndo_set_mac_address = demin_set_mac_address,
  .ndo_do_ioctl = demin_ioctl,
};

// void ether_setup(struct net_device *dev)

static void mySetup(struct net_device *dev) {
  ether_setup(dev);
  // initial mac address
  dev->dev_addr[0]=0x00;
  dev->dev_addr[1]=0x26;
  dev->dev_addr[2]=0x32;
  dev->dev_addr[3]=0xf0;
  dev->dev_addr[4]=0x20;
  dev->dev_addr[5]=0x2c;
  //
}


/************************************/
//static struct phy_device *Phy=NULL;
//static char phy_name[128];


/************************************/
/*
static void adjust_link(struct net_device *dev) {
  
}
*/
/************************************/

//static struct mii_bus *mi=NULL;

/*
static void myPhyInit(void) {
  mi=mdiobus_alloc_size(0);
  __mdiobus_register(mi,THIS_MODULE);
}
*/

/***********************************/

static struct resource demin_res[] = {
    { .start = 0x40c00000,
      .end = 0x40c00000+0x40000,
      .flags = IORESOURCE_MEM,
      .name = "io-memory"
    },
    {
      .start = 85+32,
      .end = 85+32,
      .flags = IORESOURCE_IRQ,
      .name = "irq"
    }
};


static int rc_pd=0,irq=0,rc_irq=0;

/***********************************/

static irqreturn_t my_handler(int irq_no, void* dev_id) { 
  return IRQ_NONE; // IRQ_HANDLED
}

/***********************************/
int demin_init_module (void)
{ int result;
  struct resource *r;
  //console_loglevel = CONSOLE_LOGLEVEL_MOTORMOUTH;
  myConInit();
  out("trying init demin...]");
  //
  pd=platform_device_alloc("demin",-1);
  pd->resource=&demin_res[0];
  pd->num_resources=ARRAY_SIZE(demin_res);
  //dma_set_coherent_mask(&(pd->dev),0xffffffff);
  dma_set_coherent_mask(&(pd->dev),DMA_BIT_MASK(32));
  tester=dma_alloc_coherent(&(pd->dev),4096,&DM,GFP_ATOMIC);
  if(tester!=NULL) {
    sprintf(prtbuf,"DMA buffer at bus addr %08X virt addr %lX]",DM,(u_long)tester);
    out(prtbuf);
  } else {
     sprintf(prtbuf,"No DMA buffer allocated]"); 
  };
  //
  rc_pd=platform_device_add(pd);
  sprintf(prtbuf,"demin: platform device registered rc=%d]",rc_pd);
  out(prtbuf);
  //
  demin=alloc_etherdev(0); mySetup(demin);
  demin->netdev_ops=&demin_ops;
  if ((result = register_netdev (demin))) {
    sprintf(prtbuf,"demin: Error %d  initializing demin card]",result);
    out(prtbuf);
    return result;
  }
  demin->reg_state=NETREG_REGISTERED;
  //Phy = phy_connect(demin, phy_name, &adjust_link,PHY_INTERFACE_MODE_MII);
  //myPhyInit();
  //
  /*
  if(Phy==ERR_PTR(-ENODEV)) {
    printk(KERN_ERR "demin: PHY is not connected\n");
    Phy=NULL;
  } else {
    printk(KERN_INFO "demin: PHY is connected\n");
    phy_start(Phy);
  };
  */
   r=platform_get_resource(pd,IORESOURCE_IRQ,0);
   if(r!=NULL) {
     irq=r->start;
     sprintf(prtbuf,"demin: irq from res  is %d]", irq);
     out(prtbuf);
     rc_irq=request_irq(irq,my_handler,0,DEVICE_NAME,NULL);
     sprintf(prtbuf,"demin request irq %d ,rc=%d]",irq,rc_irq);
     out(prtbuf);
   } else {
     sprintf(prtbuf,"demin: no interrupts!]");
     out(prtbuf);
   };
   //demin->flags |= IFF_UP|IFF_RUNNING;
   //netif_carrier_off(demin);
   sprintf(prtbuf,"demin: driver loaded as %s]",demin->name);
   out(prtbuf);
  return 0;
}

void demin_cleanup (void)
{ 
  sprintf(prtbuf,"demin: Cleaning Up the Module]"); out(prtbuf);
  //if(Phy!=NULL) phy_stop(Phy);
  if(rc_irq==0) free_irq(irq,NULL);
  if(tester!=NULL) dma_free_coherent(&(pd->dev),4096,tester,DM);
  unregister_netdev (demin);
  if(pd!=NULL) {
    platform_device_del(pd);
  };
  sprintf(prtbuf,"demin: Clean Up completed]");
  out(prtbuf);
}

module_init (demin_init_module);
module_exit (demin_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION("1.1");

