#include <linux/module.h>
#include <linux/printk.h>
#include <linux/kernel.h>
#include <linux/string.h>

#include <linux/err.h>

#include <linux/kthread.h>

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

#include <uapi/linux/ip.h>
#include <uapi/linux/udp.h>

#include <linux/socket.h>
#include <linux/net.h>
#include <linux/in.h>
#include <linux/udp.h>

#include <linux/dma-mapping.h>

#define DEVICE_NAME "demin"
#define DRIVER_AUTHOR "miketerrible@gmail.com"
#define DRIVER_DESC "net device module for testing"

static struct platform_device *pd=NULL;
static struct net_device *bitcoin;

static void *tester=NULL;
static dma_addr_t DM;

/************  net console     ******************/
struct socket* MySk=NULL;

static int net_err=0;
static volatile int net_bind=0;

static struct sockaddr_in a;
static unsigned char cbuf[1028];
static struct kvec kkv;
static struct iov_iter io;
static struct msghdr mh;
static unsigned char svaddr[]={ 192,168,10,100 };

#define bzero(a,n) memset(a,0,n)

static volatile int TFIN=0;

static int nget=0;
static int reuse=1;

static void getMsg(struct sk_buff* skb) {
  int rc;
  /*
  printk(KERN_INFO "bitcoin: getMsg %d %02X%02X%02X%02X%02X%02X%02X%02X\n",xlen,
    cbuf[0],cbuf[1],cbuf[2],cbuf[3],cbuf[4],cbuf[5],cbuf[6],cbuf[7]);
  */
  rc=netif_rx(skb);
  if(rc==NET_RX_SUCCESS) {
    printk(KERN_INFO "bitcoin: getMsg ok len=%d\n",skb->len);
  } else if(rc==NET_RX_DROP) {
    printk(KERN_INFO "bitcoin: getMsg dropped len=%d\n",skb->len);
  } else {
    printk(KERN_INFO "bitcoin: getMsg rc=%d len=%d\n",rc,skb->len);
  };
  //dev_kfree_skb(skb);
}


static void msgPrepare(void) {
  bzero(&io,sizeof io);
  bzero(&mh,sizeof mh);
  kkv.iov_base=&cbuf[0];
  kkv.iov_len=1024;
  io.iov_offset=0;
  io.count=1024;
  io.nr_segs=1;
  io.kvec=&kkv;
  memcpy(&mh.msg_iter,&io,sizeof io);
}

static void mySvInit(void) {
  a.sin_family = AF_INET;
  memcpy(&a.sin_addr.s_addr,svaddr,4); 
  a.sin_port = htons(1026);
  net_err=sock_create(AF_INET, SOCK_DGRAM, IPPROTO_UDP, &MySk);
  kernel_setsockopt(MySk,SOL_SOCKET,SO_REUSEADDR,(char*)&reuse,sizeof(int));  
  net_bind=kernel_bind(MySk,(struct sockaddr*)&a,sizeof a);
  if(net_bind!=0) TFIN=1;
  printk(KERN_INFO "bitcoin: bind rc=%d,TFIN=1\n",net_bind);
}

static struct task_struct *th=NULL;

static void mySvDone(void) {
  TFIN=1;
  if(net_bind!=0) return;
  printk(KERN_INFO "bitcoin: trying to shutdown server...\n");
  TFIN=2;
  //sock_release(MySk);
  printk(KERN_INFO "bitcoin: server socket released\n");
}

static void myConInit(void) { }

/*****************************/


static long ii=0;

static int tfn(void* d) {
  int rc,si;
  struct timespec now;
  struct sk_buff* skb;
  struct ethhdr* ethh;
  si=1024+sizeof(struct ethhdr);
  skb=netdev_alloc_skb(bitcoin,si);
  ethh=(struct ethhdr*)skb_push(skb,sizeof(struct ethhdr));
  ethh->h_proto = htons(ETH_P_IP);
  //skb->protocol = htons(ETH_P_IP);
  skb->protocol = eth_type_trans(skb, skb->dev);
  skb->ip_summed = CHECKSUM_UNNECESSARY;
  memcpy (ethh->h_dest, bitcoin->dev_addr, ETH_ALEN);
  skb_put(skb,1024);
  //skb_unlink(skb,skb->list);
  printk(KERN_INFO "bitcoin: recflow running\n");  
  for(;;) {
    if(TFIN!=0) {
      printk(KERN_INFO "bitcoin: recflow TFIN is %d\n",TFIN);
      kernel_sock_shutdown(MySk,SHUT_RD);
      sock_release(MySk); MySk=NULL;
      dev_consume_skb_any(skb); 
      break;
    };
    msgPrepare();
    rc=sock_recvmsg(MySk,&mh,1024,MSG_WAITALL);
    if(rc>0) { 
      //struct udphdr* udph;
      //struct iphdr *iph;
      //skb_put(skb,rc);
      skb->len=rc; memcpy(skb->data,cbuf,rc);
      getnstimeofday(&now);
      skb->tstamp = timespec_to_ktime(now);
      getMsg(skb);
    }
    ii++;
    if(ii==512000) {
      ii=0; printk(KERN_INFO "bitcoin: recvflow\n");
    };
  };
  if(skb!=NULL) dev_consume_skb_any(skb);
  printk(KERN_INFO "bitcoin: recvflow ended\n"); do_exit(0);
  return 0;
}

static void simulateIRQ(void) {
  ii=0; nget=0;
  th = kthread_create(tfn,NULL,"recvflow");
  if(th!=NULL) {
    wake_up_process(th);
    printk(KERN_INFO "bitcoin: recvflow: simulate IRQ started\n");
  };
}

/*****************************/

static int bitcoin_ioctl(struct net_device *dev,struct ifreq *ifr, int cmd) {
  printk(KERN_INFO "bitcoin: do_ioctl cmd=%x\n",cmd);
  return 0;
}

static int bitcoin_open (struct net_device *dev)
{ 
  printk("bitcoin: bitcoin_open called (ip link set eth1 up)\n");
  netif_start_queue (dev);
  simulateIRQ();
  return 0;
}

static int bitcoin_release (struct net_device *dev)
{ printk(KERN_INFO "bitcoin: bitcoin_release called (ip link set eth1 down)\n");
  netif_stop_queue(dev);
  return 0;
}


static int bitcoin_xmit (struct sk_buff *skb,struct net_device *dev)
{ dev->trans_start=jiffies;
  printk(KERN_INFO "bitcoin: %lu dummy xmit function called....\n",dev->trans_start);
  dev_kfree_skb(skb);
  return NETDEV_TX_OK;
}

static int bitcoin_set_mac_address(struct net_device *ndev,void *address) {
  //memcpy(ndev->dev_addr,address+2,6);
  return eth_mac_addr(ndev,address);
}

static int bitcoin_init(struct net_device *dev) {
  //dev->flags=IF_OPER_UP|IFF_UP|IFF_BROADCAST;
  dev->state=__LINK_STATE_START|__LINK_STATE_PRESENT;
  //dev->flags = IFF_UP;
  //netif_carrier_off(demin);
  //dev->flags |= IFF_UP|IFF_RUNNING;
  //dev->flags |= IF_OPER_UP|IFF_UP;
  dev->flags |= IFF_NOARP|IFF_UP|IFF_RUNNING;
  dev->operstate |= IF_OPER_UP;
  printk(KERN_INFO "bitcoin: init\n");
  return 0;
}

static void bitcoin_uninit(struct net_device *dev) {
  printk(KERN_INFO "bitcoin: uninit\n"); 
  dev->state = 0;
  dev->flags = 0;
}

static struct net_device_ops bitcoin_ops= {
  .ndo_init         = bitcoin_init,
  .ndo_uninit       = bitcoin_uninit,
  .ndo_open         = bitcoin_open,
  .ndo_stop         = bitcoin_release,
  .ndo_start_xmit   = bitcoin_xmit,
  .ndo_set_mac_address = bitcoin_set_mac_address,
  .ndo_do_ioctl = bitcoin_ioctl,
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

static struct resource bitcoin_res[] = {
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
int bitcoin_init_module (void)
{ int result;
  struct resource *r;
  //console_loglevel = CONSOLE_LOGLEVEL_MOTORMOUTH;
  myConInit();
  printk(KERN_INFO "bitcoin: trying init...\n");
  mySvInit();
  //
  pd=platform_device_alloc("bitcoin",-1);
  pd->resource=&bitcoin_res[0];
  pd->num_resources=ARRAY_SIZE(bitcoin_res);
  //dma_set_coherent_mask(&(pd->dev),0xffffffff);
  dma_set_coherent_mask(&(pd->dev),DMA_BIT_MASK(32));
  tester=dma_alloc_coherent(&(pd->dev),4096,&DM,GFP_ATOMIC);
  if(tester!=NULL) {
    printk(KERN_INFO "bitcoin: DMA buffer at base addr %08X virt addr %lX\n",DM,(u_long)tester);
  } else {
    printk(KERN_INFO "bitcoin: No DMA buffer allocated\n");
  };
  //
  rc_pd=platform_device_add(pd);
  printk(KERN_INFO "bitcoin: platform device registered rc=%d\n",rc_pd);
  //
  bitcoin=alloc_etherdev(0); mySetup(bitcoin);
  bitcoin->netdev_ops=&bitcoin_ops;
  if ((result = register_netdev (bitcoin))) {
    printk(KERN_INFO "bitcoin: Error %d  initializing bitcoin card\n",result);
    return result;
  }
  bitcoin->reg_state=NETREG_REGISTERED;
  r=platform_get_resource(pd,IORESOURCE_IRQ,0);
  if(r!=NULL) {
    irq=r->start;
    printk(KERN_INFO "bitcoin: irq from res  is %d\n", irq);
    rc_irq=request_irq(irq,my_handler,0,DEVICE_NAME,NULL);
    printk(KERN_INFO "bitcoin request irq %d ,rc=%d\n",irq,rc_irq);
  } else {
     printk(KERN_INFO "bitcoin: no interrupts!\n");
  };
  printk(KERN_INFO "bitcoin: driver loaded as %s\n",bitcoin->name);
  return 0;
}

void bitcoin_cleanup (void)
{ 
  printk(KERN_INFO "bitcoin: Cleaning Up the Module\n"); 
  mySvDone();
  if(rc_irq==0) free_irq(irq,NULL);
  if(tester!=NULL) { dma_free_coherent(&(pd->dev),4096,tester,DM); tester=NULL; };
  unregister_netdev(bitcoin);
  if(pd!=NULL) {
    platform_device_del(pd); pd=NULL;
  };
  printk(KERN_INFO "bitcoin: Clean Up completed\n");
}

module_init (bitcoin_init_module);
module_exit (bitcoin_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION("1.1");

