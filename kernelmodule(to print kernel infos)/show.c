#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/multi_pnfs.h>
#include <linux/spinlock_types.h>
#include <linux/rwlock_types.h>

extern  struct mlt *mlt_client;
extern  rwlock_t mlt_client_rwlock;


static int mltinfo_proc_show(struct seq_file *m, void *v)
{
	int i;
	struct list_head *dlist = NULL;
	struct list_head *mlist = NULL;
	struct mlt_dirname *mlt_dir;
	struct mst *mst_ent;
	struct mst *entry_mst;
	struct list_head *ptr;
	struct mlt *mlt = mlt_client;
	read_lock(&mlt_client_rwlock);
	if (mlt == NULL)
	{
			seq_printf(m, "mlt is null\n");
			read_unlock(&mlt_client_rwlock);
			return 0;
	}
	seq_printf(m,"version = %u\t""masterid = %u\n",mlt->version, mlt->master_id);
	for (i = 0;i<MLT_BUCKET_COUNT;i++){
		seq_printf(m,"bucket%d:\t""mds_id1 = %u\t""mds_id2 = %u\t""mds_id3 = %u\n""local_usr_id%u\n",
						i,mlt->buckets[i].mds_id1,mlt->buckets[i].mds_id2,mlt->buckets[i].mds_id3,
						mlt->buckets[i].local_use_id);
       		 if(mlt->buckets[i].dir_head.next != &( mlt->buckets[i].dir_head) ){
           	 //printk("sjl: want dir\n");
          	  for(dlist = mlt->buckets[i].dir_head.next;dlist != &(mlt->buckets[i].dir_head); dlist=dlist->next){
               		 mlt_dir = container_of(dlist,struct mlt_dirname, dir_list);
                     seq_printf(m,"---->dir_state = %u\t""dirname_len = %u\t""dirname = %s\n",mlt_dir->dir_state,mlt_dir->dirname_len,mlt_dir->dirname);
              }
            }

	}
	list_for_each(ptr, &(mlt->mst_head)){
                entry_mst = list_entry(ptr, struct mst, mst_list);
                seq_printf(m, "@@@@ mds_id=%u\t"" mds_ip=%s\t""mds_state=%u\t  @@\n", entry_mst->mds_id, entry_mst->mds_ip, entry_mst->mds_state);
        }
	/*
	if ( mlt->mst_head.prev == mlt->mst_head.next )
	{
		seq_printf(m, "mst_head.prev  ==  mst_head.next\n");
	}
	if (mlt->mst_head.prev != mlt->mst_head.next ){			
		for(mlist = mlt->mst_head.next;mlist!= &(mlt->mst_head);mlist = mlist->next){
			mst_ent = container_of(mlist,struct mst, mst_list);
			seq_printf(m, "mds_id:%u\t"" mds_ip:%s\t"" mds_state:%u\n",mst_ent->mds_id,mst_ent->mds_ip,mst_ent->mds_state);
		}
	
	}
	*/
	read_unlock(&mlt_client_rwlock);
	return 0;
}

static int mltinfo_proc_open(struct inode *inode, struct file *file)
{
		return single_open(file, mltinfo_proc_show, NULL);
}

static const struct file_operations mltinfo_proc_fops =
{
		.open    = mltinfo_proc_open,
		.read    = seq_read,
		.llseek	 = seq_lseek,
		.release = single_release,
};

static int print_module_init(void)
{
	struct proc_dir_entry *entry;
	
	entry = create_proc_entry("fs/nfsfs/mltinfo", 0, NULL);
	if (!entry){
		return -ENOMEM;
	}
	entry->proc_fops = &mltinfo_proc_fops;
	return 0;
}

static void print_module_exit(void)
{
		remove_proc_entry("fs/nfsfs/mltinfo", NULL);
}
module_init(print_module_init);
module_exit(print_module_exit);

MODULE_LICENSE("GPL");
