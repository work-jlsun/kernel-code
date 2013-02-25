#ifndef __LOAD_XDR_H
#define __LOAD_XDR_H
#include <linux/types.h>
#include "load.h"

/*procedure number*/
#define GETLOAD 11
#define GET_RRD_IP 1
/*buffer len*/
#define GETLOAD_BUF_LEN  1024
#define GETRRD_IP_BUF_LEN  40

/*export for loadxdr.c*/
extern struct rpc_procinfo mds_load_procedures_version1[];
extern struct rpc_version mds_load_version1;

extern struct rpc_procinfo rdd_ip_get_procedures_version1[];
extern struct rpc_version rdd_ip_get_version1;


/*args and result*/
#define MAX_MDS 40 
struct mds_load_getload_args 
{ 
		 char s[800]; 
		 u32 len; 
}; 
struct mds_load_getload_res 
{ 
	  u32 mds_num; 
	  struct mds_load_t   mds_load[MAX_MDS]; 
};

struct rrd_ip_get_args
{
		int data;
};

struct rrd_ip_get_res
{
		char rrd_ip[MAX_RRD_IP_LENGTH];		
};

#endif
