#include "load.h"
#include "loadxdr.h"
struct rpc_version  *mds_load_version[2] =
{
    [1]       =  &mds_load_version1,
};

struct rpc_stat   mds_load_rpcstat;

struct rpc_program mds_load_program =
{
    .name           = "mds_load",
    .number         = MDS_LOAD_PROGRAM,
    .nrvers         = ARRAY_SIZE(mds_load_version),
    .version        = mds_load_version,
    .stats          = &mds_load_rpcstat,
//  .pipe_dir_name      = "/nfs",
};
/*******for get the rrd server*****************************/

struct rpc_version  *rrd_ip_get_version[2] = 
{
		[1]    = &rdd_ip_get_version1,
};
struct rpc_stat   rrd_ip_get_rpcstat;

struct rpc_program rrd_ip_get_program = 
{
    .name           = "rrd_ip_get",
	.number         = RRD_IP_GET_PROGRAM,
	.nrvers         = ARRAY_SIZE(mds_load_version),
	.version        = rrd_ip_get_version,
	.stats          = &rrd_ip_get_rpcstat,
};







/***************************************/
static void load_init_timeout_value(int protocol, struct rpc_timeout *to)
{
	printk("load:-->%s\n",__func__);
        switch (protocol){
                case IPPROTO_TCP:
                case IPPROTO_UDP:
                            to->to_initval      = 6000;
                            to->to_maxval       = 18000;
                            to->to_increment    = 6000;
                            to->to_retries      = 6;
                            to->to_exponential  = 0;
                            break;
        }
	printk("load:<--%s\n",__func__);
}


static int load_create_rpc_client( struct load_client *lclnt, const struct rpc_timeout
                                  *timeparms, rpc_authflavor_t flavor, int program_num)
{
    struct rpc_clnt *clnt = NULL;

	struct rpc_program  *program;

	printk("load:-->%s\n",__func__);
	switch (program_num)
	{
			case MDS_LOAD_PROGRAM :
									program = &mds_load_program;
									break;
			case RRD_IP_GET_PROGRAM :
									program = &rrd_ip_get_program;
									 break;
			default : 				return -1;
	}
    struct rpc_create_args args = {
            .protocol = lclnt->protocol,
            .address  = (struct sockaddr *)lclnt->address,
            .addrsize = sizeof(struct sockaddr_in),
            .timeout  = timeparms,
            .servername = lclnt->cl_hostname,
            .program  = program,
            .version  = 1,
            .authflavor = flavor,
            //.flags        = RPC_CLNT_CREATE_NOPING,

    };

    clnt = rpc_create(&args);
    if(IS_ERR(clnt)){
        printk("-->%s:cannot create rpc client.Error = %ld\n",__func__, PTR_ERR(clnt));
		return PTR_ERR(clnt);
    }
    lclnt->clnt = clnt;

	printk("load:<--%s\n", __func__);
    return 0;
}

int load_free_client(struct load_client *lclnt) 
{

	printk("load:-->%s\n", __func__);
	if (lclnt == NULL){
			printk("load-->%s,null pointer\n",__func__);
			return 0;
	}
	if (lclnt->address){
			kfree(lclnt->address);
	}
	if (lclnt->cl_hostname){
			printk("load-->%s,free: %s",__func__, lclnt->cl_hostname);
			kfree(lclnt->cl_hostname);
	}
	if (!IS_ERR(lclnt->clnt)){
			rpc_shutdown_client(lclnt->clnt);
	}
	kfree(lclnt);
	printk(" ok !\n");
	printk("load:<--%s\n", __func__);
    return 0; 
}

/**********************************************
 *author:sjl
 *function:free the form load_client 
            include the task and kill
            the rpc
 *
 *
 * *******************************************/
int load_free_former(struct load_current *cload)
{
	
	printk("load:-->%s\n", __func__);
	if( load_thread_exit != TURE && cload->clnt_task){
            kthread_stop(cload->clnt_task);
			cload->clnt_task = NULL;
    }
    load_free_client(cload->lclnt);
	cload->lclnt = NULL;

	printk("load:<--%s\n", __func__);
	return 0;
}



int load_init_client(int protocol, char* ip, struct load_client *lclnt, int program_num)
{
        //struct load_client  lclnt;
		int ret;
        struct rpc_timeout timeparms;
        struct sockaddr_in *server_addr;
		printk("load:-->%s\n", __func__);
        load_init_timeout_value(IPPROTO_TCP,&timeparms);
        server_addr = (struct sockaddr_in*)kmalloc(sizeof(struct sockaddr_in),GFP_KERNEL);
        if(!server_addr){
            BUG();
        }
        memset(server_addr, 0, sizeof(server_addr));
        server_addr->sin_family = AF_INET;
        server_addr->sin_addr.s_addr = in_aton(ip); 

        lclnt->protocol = protocol;
        lclnt->address  = server_addr; 
        lclnt->cl_hostname = kstrdup(ip,GFP_KERNEL); 
        if(!lclnt->cl_hostname){
            printk("-->%s,kstrdup error\n",__func__);
            BUG();
        }
        lclnt->clnt = NULL;
        if( ( ret = load_create_rpc_client(lclnt, &timeparms, RPC_AUTH_NULL,program_num) ) != 0 )
        {
                printk("load-->%s return error\n",__func__);
                return ret;
        }

		printk("load:<--%s\n", __func__);
		return 0;
}




