#include "load.h"
#include "loadxdr.h"
#include <linux/poll.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/fsnotify.h>
#include <linux/syscalls.h>
#include <linux/multi_pnfs.h>
#include <linux/spinlock_types.h>
#include <linux/rwlock_types.h>
#include <linux/nfs_fs.h>
#include <linux/completion.h>
#include <linux/jiffies.h>
//#include <linux/fs/nfs/nfs4_fs.h>
MODULE_LICENSE("GPL");

//bool  change_task_shoud_exit = 0;
bool  load_thread_exit = FAULT;
extern  struct mlt *mlt_client;
extern  rwlock_t mlt_client_rwlock;

extern struct completion   load_completion;

extern struct nfs_mds_load mds_loads;
extern  rwlock_t  mds_loads_lock;

struct load_current  current_server;

struct task_struct  *load_task; 
char loadip[20] = "192.168.0.162";


struct mds_load_getload_res  load_res;


/**********************************************
 *author:sjl
 *function:choose the master mds ip form mlt
 *  
 *
 *
 *
 * *******************************************/

static void load_choose_mainmds_ip(char *ip)
{

    u32 master_id;
    struct list_head *mst_head;
    struct mst *ptr_mst;
    struct mst *next_mst;
    master_id = mlt_client->master_id;
    mst_head =  &(mlt_client->mst_head);
    list_for_each_entry_safe(ptr_mst, next_mst, mst_head, mst_list){
            if ( ptr_mst->mds_id == master_id){
                    printk("load:-->the master mds ip is %s\n",ptr_mst->mds_ip);
                    strcpy(ip, ptr_mst->mds_ip);
                    break;
            }

    }
}

/**********************************************
 *author:sjl
 *function: calculate all the mds load value
 *  
 *
 *
 *
 * *******************************************/
void  load_calculate_all(struct mds_load_getload_res *res)
{
	int mds_num;
	int i;
	int io_used;
	int net_used;
	struct mst *mst_entry;
	struct mst *mst_next;
	printk("load:-->%s\n",__func__);
	mds_num = res->mds_num;

	read_lock(&mlt_client_rwlock);
	write_lock(&mds_loads_lock);
	mds_loads.mds_num = 0;
	for (i = 0; i != mds_num; i++){		
		printk("load ip:%s\n", res->mds_load[i].mds);
		list_for_each_entry_safe(mst_entry, mst_next, &(mlt_client->mst_head) ,mst_list){
			if( !strcmp(mst_entry->mds_ip, res->mds_load[i].mds) ){
				
				mds_loads.loads[mds_loads.mds_num].mds_id = mst_entry->mds_id;	
				io_used = (res->mds_load[i].ior + res->mds_load[i].iow)*100/IO_MAX;
				net_used = (res->mds_load[i].neti + res->mds_load[i].neto)*100/NET_MAX;
				if(res->mds_load[i].cpu > CPU_MAX){
					mds_loads.loads[mds_loads.mds_num].load = 100;	
				}else{
					mds_loads.loads[mds_loads.mds_num].load =( res->mds_load[i].cpu * R_CPU_LOAD_ARG
								+ res->mds_load[i].mem * R_MEM_LOAD_ARG
								+ res->mds_load[i].disk * R_DISK_LOAD_ARG
								+ io_used * R_IO_LOAD_ARG
								+ net_used * R_NET_LOAD_ARG ) / 100;
				}
				mds_loads.mds_num++;
				printk("mds id%u: load = %u \n", mds_loads.loads[i].mds_id, mds_loads.loads[i].load);
				break;
			}
		}
	}
	mds_loads.last_update_time = get_jiffies_64();	
	write_unlock(&mds_loads_lock);
	read_unlock(&mlt_client_rwlock);
	printk("load:<--%s\n",__func__);
}


/*
 *sjl:patch all the mds ip as an parameter of an rpc call
 *
 */
void mds_pack_all_mdsip(char * ip)
{
	struct mst *mst_entry;
	struct mst *mst_next;
	char *ips = ip;
	int len;
	read_lock(&mlt_client_rwlock);
	list_for_each_entry_safe(mst_entry, mst_next, &(mlt_client->mst_head) ,mst_list){
			len = strlen(mst_entry->mds_ip);
			strncpy(ips,mst_entry->mds_ip, len);
			ips += len;
			strncpy(ips, ":",1);
			ips++;
	}
	*(--ips) = '\0';
	read_unlock(&mlt_client_rwlock);
}

/***up layer procs*/
static int mds_load_proc_getload(struct load_client *server)
{
	int status = -1;
	struct mds_load_getload_args args;
	mds_pack_all_mdsip(args.s);
	args.len = strlen(args.s);
	struct rpc_message msg = {
		.rpc_proc = &mds_load_procedures_version1[GETLOAD],
		.rpc_argp = (void *)&args,
		.rpc_resp = (void *)&load_res,
	};
	status = rpc_call_sync(server->clnt, &msg, 0);
	if(status){
			printk("load-->err occur while rpc_call_sync!\n");
			return 0;
	}
	printk("load:sjl recv load ok\n");
	load_calculate_all(&load_res);
	return 0;
}

/***get the rrd server ip*/
static int rrd_proc_getip(struct load_client *server, char *ip)
{
	int status = -1;
	struct rrd_ip_get_args args = {
		.data = 10,
	};
	struct rrd_ip_get_res res;
	struct rpc_message msg = {
		.rpc_proc = &rdd_ip_get_procedures_version1[GET_RRD_IP],
		.rpc_argp = (void *)&args,
		.rpc_resp = (void *)&res,
	};
	status = rpc_call_sync(server->clnt, &msg, 0);
	if(status){
			printk("load-->err occur while rpc_call_sync!\n");
			return 0;
	}
	printk("load:sjl recv ip:%s ok\n", res.rrd_ip);
	strncpy(ip, res.rrd_ip, strlen(res.rrd_ip) + 1);
	return 0;
}

/******here is the application and kernel thread *************************/

static int heart_beat_load_get( void *arg)
{
	struct load_client *lclnt = (struct load_client *)arg;
	while(!kthread_should_stop()){
			schedule_timeout_uninterruptible(50000);
			mds_load_proc_getload(lclnt);
	}
	return 0;
}
/****************************************************
 *author:sjl
 *function: when the client mount start  get load
 *
 *
 *
 * **************************************************/
static int  load_start_getload(void *args)
{
	int status;
	//char *ip = (char *)args;
	struct load_client *load_clnt = (struct load_client *)kmalloc(sizeof(struct load_client), GFP_KERNEL);
	
	printk("load:-->%s\n", __func__);
	if(!load_clnt){
		printk("load:-->kmalloc error\n");
		return -1;
	}
	memset(load_clnt, 0, sizeof(struct load_client));
	printk("load:--ip:%s\n", loadip);
	status = load_init_client(IPPROTO_TCP, loadip, load_clnt, MDS_LOAD_PROGRAM);
	if(status != 0){
		printk("load:-->%s\n",__func__);
		load_thread_exit = TURE;
		return -1;
	}	
	current_server.lclnt = load_clnt;

	while(!kthread_should_stop()){
			schedule_timeout_uninterruptible(3*HZ);
			mds_load_proc_getload(current_server.lclnt);
	}
	printk("load:<--%s\n", __func__);
	return 0;
}


/****************************************************
 *author:sjl
 *function: when the client mount start  get load
 *
 *
 *
 * **************************************************/
static int load_get_rrd_ip(void * args)
{
	int status;
	char ip[20];
	char testip[20] = "192.168.0.162";
	struct task_struct *task = NULL;	
	struct load_client *load_clnt = (struct load_client *)kmalloc(sizeof(struct load_client), GFP_KERNEL);
	
	printk("laod:-->%s\n", __func__);
	if(!load_clnt){
		printk("load:-->kmalloc error\n");
		return -1;
	}
	memset(load_clnt, 0, sizeof(struct load_client));
	while(1){
			if(!mlt_client){
				printk("laod:--mlt is null,wait\n");
				schedule_timeout_uninterruptible(3*HZ);
			}else{
				break;
			}
			if(kthread_should_stop()){
				//	unlock_kernel();
					return 0;
			}
	}
	printk("load:-->ok have got the mlt_client\n");
	read_lock(&mlt_client_rwlock);
	load_choose_mainmds_ip(ip);
	read_unlock(&mlt_client_rwlock);
	printk("load:-->ip:%s\n", ip);

	status = load_init_client(IPPROTO_TCP, ip, load_clnt, RRD_IP_GET_PROGRAM);
	if (status != 0){
		printk("load:-->load_init_client\n");
		return -1;
	}
	rrd_proc_getip(load_clnt,ip);
	printk("load:--the rrd_server ip:%s\n", ip);
	load_free_client(load_clnt);

	strncpy(loadip, ip, strlen(ip)+1);  //just test here
	load_task = kthread_run(load_start_getload, ip,"first_thread");
	if(IS_ERR(task)){
		printk("load:-->%s,kthread_run 1 error\n",__func__);
		return -1;
	}
	current_server.clnt_task = load_task;

	printk("laod:<--%s\n", __func__);
	return 0;
}


static int load_module_init(void)
{
	int status;
	struct load_client load_clnt;
	
	
	struct task_struct *task = NULL;	
	task = kthread_run(load_get_rrd_ip,task,"first_thread");
	if(IS_ERR(task)){
		printk("load:-->%s,kthread_run 1 error\n",__func__);
		return -1;
	}
	/*
	current_server.clnt_task = task;
	
	task = kthread_run(load_change_getload,task,"second_thread");
	if(IS_ERR(task)){
		printk("load:-->%s,kthread_run 2 error\n",__func__);
		return -1;
	}
	change_task = task;
	*/
	
	return 0;
}


static void load_module_exit(void)
{
	
		load_free_former(&current_server);
}

module_init(load_module_init);
module_exit(load_module_exit);





