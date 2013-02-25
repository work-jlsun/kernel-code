#ifndef __LOAD_H
#define __LOAD_H


#include <linux/init.h>
#include <linux/module.h>
#include <linux/socket.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/stat.h>
#include <linux/errno.h>
#include <linux/unistd.h>
#include <linux/sunrpc/clnt.h>
#include <linux/sunrpc/stats.h>
#include <linux/sunrpc/metrics.h>
#include <linux/sunrpc/xprtsock.h>
#include <linux/sunrpc/xprtrdma.h>
#include <linux/lockd/bind.h>
#include <linux/seq_file.h>
#include <linux/smp_lock.h>
#include <linux/mount.h>
#include <linux/vfs.h>
#include <linux/inet.h>
#include <linux/in6.h>
#include <net/ipv6.h>

#include <linux/kthread.h>
#include <linux/freezer.h>
#include <linux/sched.h>

/*this are the proportion of all kinds of load*/
#define R_CPU_LOAD_ARG		0
#define R_MEM_LOAD_ARG		5
#define R_NET_LOAD_ARG		70
#define R_IO_LOAD_ARG		25
#define R_DISK_LOAD_ARG		0
/**/
#define IO_MAX			200000000
#define NET_MAX			120000000
#define CPU_MAX			95
#define MEM_MAX			99
#define DISK_MAX		95

#define MAX_RECONNECT_TRY 3
#define MAX_MDS_NAME 20 

#define MAX_RRD_IP_LENGTH 32

/*************for the rpc program num*****************/
#define MDS_LOAD_PROGRAM  0x20001001
#define RRD_IP_GET_PROGRAM 0x20110520

struct mds_load_t{ 
    char mds[MAX_MDS_NAME]; 
    u32 mds_id;
	u32 cpu; 
    u32 mem; 
    u32 ior; 
    u32 iow; 
    u32 neti; 
    u32 neto; 
    u32 disk; 
    struct list_head   mds_list; 
}; 
 
struct all_mds_load_t{ 
    u32 mds_num; 
    rwlock_t  load_list_lock;            
    struct list_head  all_mds; 
}; 
/******************************/ 
 
 
struct load_client 
{ 
    int protocol;
	u32 version;       /*same as the mlt version*/
    struct sockaddr_in  *address; 
    char *cl_hostname; 
    struct rpc_clnt  *clnt; 
};

struct load_current
{
	struct load_client *lclnt;
	struct task_struct *clnt_task;
};

/****export for loadclient.c*/
#define TURE 1
#define FAULT 0
extern bool  load_thread_exit;
extern int load_init_client(int, char*, struct load_client *, int);
extern int load_free_former(struct load_current *);
extern int load_free_client(struct load_client *);
#endif
