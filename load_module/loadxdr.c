#include "loadxdr.h"
#include "load.h"
static __be32 *reserve_space(struct xdr_stream *xdr, size_t nbytes){
	 __be32 *p = xdr_reserve_space(xdr,nbytes);
	return p;
}


static void encode_getload(struct xdr_stream *xdr,
				                                const struct mds_load_getload_args *args)
{
		    u32 len = args->len;
			__be32 *p;
			 printk("load--- %s,the arg len is %u,s is %s\n",__func__,len,args->s);
			 p = reserve_space(xdr, 4 + len);
			 *p++ =cpu_to_be32(len);
			 xdr_encode_opaque_fixed(p,args->s, len);
}

static int mds_load_enc_getload(struct rpc_rqst *req, __be32 *p,
				                                const struct mds_load_getload_args *args)
{
		    struct xdr_stream xdr;
			 printk("load-->%s\n", __func__);
			 xdr_init_encode(&xdr,&req->rq_snd_buf, p);
			 encode_getload(&xdr,args);
			 printk("load<--%s\n", __func__);
			 return 0;
}


static int decode_getload(struct xdr_stream *xdr, struct mds_load_getload_res *res)
{
    __be32 *p;
    int i;

    printk("load:-->%s\n",__func__);
    p = xdr_inline_decode(xdr,4);
    res->mds_num = be32_to_cpup(p);
	if (res->mds_num > MAX_MDS){
			printk("mds number > 40 , load cutted\n");
			res->mds_num = MAX_MDS;
	}
    printk("the ds_num is %u\n",res->mds_num);
    for (i = 0;i<res->mds_num;i++){
        p = xdr_inline_decode(xdr,MAX_MDS_NAME + 28);
        memcpy(res->mds_load[i].mds,p,MAX_MDS_NAME);
        p += XDR_QUADLEN(MAX_MDS_NAME);
        //res->mds_load[i].mds_id = be32_to_cpup(p++);
        res->mds_load[i].cpu = be32_to_cpup(p++);
        res->mds_load[i].mem = be32_to_cpup(p++);
        res->mds_load[i].ior = be32_to_cpup(p++);
        res->mds_load[i].iow = be32_to_cpup(p++);
        res->mds_load[i].neti = be32_to_cpup(p++);
        res->mds_load[i].neto = be32_to_cpup(p++);
        res->mds_load[i].disk = be32_to_cpup(p);
        printk("mds_id%d: cpu:%u, mem:%u, ior:%u, iow:%u, neti:%u, neto:%u, disk:%u\n",
                res->mds_load[i].mds_id,res->mds_load[i].cpu,res->mds_load[i].mem,res->mds_load[i].ior,res->mds_load[i].iow,
                res->mds_load[i].neti,res->mds_load[i].neto,res->mds_load[i].disk);
    }
    return 0;
}


static int mds_load_dec_getload(struct rpc_rqst *rqstp, __be32 *p,
                                                struct mds_load_getload_res *res)
{
        struct xdr_stream xdr;
        xdr_init_decode(&xdr, &rqstp->rq_rcv_buf,p);
        decode_getload(&xdr,res);
        return 0;
}

struct rpc_procinfo mds_load_procedures_version1[] =
{
    [GETLOAD]  = {
        .p_proc     = GETLOAD,
        .p_encode   = (kxdrproc_t)mds_load_enc_getload,
        .p_decode   = (kxdrproc_t)mds_load_dec_getload,
        .p_arglen   = GETLOAD_BUF_LEN,
        .p_replen   = GETLOAD_BUF_LEN,
        .p_statidx  = GETLOAD,
        .p_name     = "mds_getload",
    },
};

struct rpc_version mds_load_version1 =
{
        .number     = 1,
        .nrprocs    = ARRAY_SIZE(mds_load_procedures_version1),
        .procs      = mds_load_procedures_version1,
};

/***********************************************
 *author:sjl
 *function:the following code are for get rrdserver
 *       ip form mds 
 *
 *
 * *******************************************/
static void encode_getip(struct xdr_stream *xdr,  const struct rrd_ip_get_args *args)
{
		    u32 len = args->data;
			__be32 *p;
			 printk("load--- %s,the arg len is %u\n",__func__,len);
			 p = reserve_space(xdr, 4);
			 *p++ =cpu_to_be32(len);
}

static int rrd_ip_enc_get(struct rpc_rqst *req, __be32 *p, const struct rrd_ip_get_args *args)
{
		    struct xdr_stream xdr;
			 printk("load-->%s\n", __func__);
			 xdr_init_encode(&xdr,&req->rq_snd_buf, p);
			 encode_getip(&xdr,args);
			 printk("load<--%s\n", __func__);
			 return 0;
}









static int decode_getip(struct xdr_stream *xdr, struct rrd_ip_get_res *res)
{
	__be32 *p;
	printk("load:-->%s\n",__func__);
	p = xdr_inline_decode(xdr,MAX_RRD_IP_LENGTH);
	memcpy(res->rrd_ip,p,MAX_RRD_IP_LENGTH);
	printk("load:--ip:%s\n", res->rrd_ip);
	printk("load:<--%s\n",__func__);
	return 0;
}
static int rrd_ip_dec_get(struct rpc_rqst *rqstp, __be32 *p, struct rrd_ip_get_res *res)
{
	struct xdr_stream xdr;
	 xdr_init_decode(&xdr, &rqstp->rq_rcv_buf,p);
	 decode_getip(&xdr,res);
	 return 0;
}
struct rpc_procinfo rdd_ip_get_procedures_version1[] =
{
    [GET_RRD_IP]  = {
        .p_proc     = GET_RRD_IP,
        .p_encode   = (kxdrproc_t)rrd_ip_enc_get,
        .p_decode   = (kxdrproc_t)rrd_ip_dec_get,
        .p_arglen   = GETRRD_IP_BUF_LEN,
        .p_replen   = GETRRD_IP_BUF_LEN,
        .p_statidx  = GET_RRD_IP,
        .p_name     = "get_rdd_ip",
    },
};




struct rpc_version rdd_ip_get_version1 =
{
 	.number     = 1,
	.nrprocs    = ARRAY_SIZE(rdd_ip_get_procedures_version1),
	.procs      = rdd_ip_get_procedures_version1,
};
