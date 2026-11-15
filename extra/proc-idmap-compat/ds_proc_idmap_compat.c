// Public Domain since creation (AI-generated; no copyright subsists).
#include <linux/module.h>
#include <linux/cred.h>
#include <linux/seq_file.h>
#include <linux/pid.h>
#include <linux/magic.h>
#include <linux/user_namespace.h>
#include <proc/internal.h>
#include "shadow_hook.h"

/* Device-specific companion for the matching Android 6.12 proc_inode ABI.
 * Real proc dentries preserve native PID identity, permissions and revalidation.
 * This module is boot-lifetime: cached proc inodes retain these fops. */
struct compat_entry {
 const char *name; unsigned int len; umode_t mode;
 const struct inode_operations *iop;
 const struct file_operations *fop;
 union proc_op op;
};
static struct user_namespace *(*ns_get)(struct user_namespace *);
static void (*ns_put)(struct user_namespace *);
static const struct seq_operations *seqs[3];
static ssize_t (*writers[4])(struct file *, const char __user *, size_t, loff_t *);
static int (*groups_show)(struct seq_file *, void *);
static struct dentry *(*instantiate)(struct dentry *, struct task_struct *, const void *);
static struct dentry *(*native_tgid)(struct inode *, struct dentry *, unsigned int);
static struct dentry *(*native_tid)(struct inode *, struct dentry *, unsigned int);
static struct compat_entry entries[4];
static int kind_for_inode(struct inode *inode)
{
 int i;
 for (i=0;i<4;i++) if (inode->i_fop==entries[i].fop) return i;
 return -EINVAL;
}
static int __nocfi map_open(struct inode *inode, struct file *file)
{
 struct task_struct *task=get_proc_task(inode);
 struct user_namespace *ns; int ret, kind=kind_for_inode(inode);
 if (!task) return -ESRCH;
 if (kind<0) { put_task_struct(task); return kind; }
 rcu_read_lock(); ns=ns_get(__task_cred(task)->user_ns); rcu_read_unlock();
 put_task_struct(task);
 if (kind==3 && (file->f_mode & FMODE_WRITE) && !ns_capable(ns,CAP_SYS_ADMIN)) {
  ns_put(ns); return -EACCES;
 }
 ret=kind==3 ? single_open(file,groups_show,ns) : seq_open(file,seqs[kind]);
 if (ret) { ns_put(ns); return ret; }
 ((struct seq_file *)file->private_data)->private=ns;
 return 0;
}
static int __nocfi map_release(struct inode *inode, struct file *file)
{
 struct user_namespace *ns=((struct seq_file *)file->private_data)->private;
 int ret=kind_for_inode(inode)==3 ? single_release(inode,file) : seq_release(inode,file);
 ns_put(ns); return ret;
}
static ssize_t __nocfi map_write(struct file *file,const char __user *buf,size_t size,loff_t *pos)
{
 int kind=kind_for_inode(file_inode(file));
 return kind<0 ? kind : writers[kind](file,buf,size,pos);
}
static const struct file_operations fops[4] = {
#define OPS { .owner=THIS_MODULE,.open=map_open,.release=map_release,.read=seq_read,.llseek=seq_lseek,.write=map_write }
 OPS,OPS,OPS,OPS
#undef OPS
};
static struct dentry *__nocfi fallback(struct inode *dir,struct dentry *dentry,struct dentry *result)
{
 struct task_struct *task; int i;
 if (!IS_ERR(result) || PTR_ERR(result)!=-ENOENT || dir->i_sb->s_magic!=PROC_SUPER_MAGIC) return result;
 for(i=0;i<4;i++) if (dentry->d_name.len==entries[i].len && !memcmp(dentry->d_name.name,entries[i].name,entries[i].len)) break;
 if(i==4) return result;
 task=get_proc_task(dir); if(!task) return result;
 result=instantiate(dentry,task,&entries[i]); put_task_struct(task);
 return result;
}
static struct dentry *__nocfi lookup_tgid(struct inode *dir,struct dentry *d,unsigned int flags)
{ return fallback(dir,d,native_tgid(dir,d,flags)); }
static struct dentry *__nocfi lookup_tid(struct inode *dir,struct dentry *d,unsigned int flags)
{ return fallback(dir,d,native_tid(dir,d,flags)); }
static const char *const tgid_names[]={"proc_tgid_base_lookup",NULL};
static const char *const tid_names[]={"proc_tid_base_lookup",NULL};
static struct shadow_hook hooks[]={
 SHADOW_HOOK(tgid_names,lookup_tgid,&native_tgid),
 SHADOW_HOOK(tid_names,lookup_tid,&native_tid)
};
static int __init compat_init(void)
{
 const char *names[]={"uid_map","gid_map","projid_map","setgroups"};
 const char *sn[]={"vns_proc_uid_seq_operations","vns_proc_gid_seq_operations","vns_proc_projid_seq_operations"};
 const char *wn[]={"vns_proc_uid_map_write","vns_proc_gid_map_write","vns_proc_projid_map_write","vns_proc_setgroups_write"};
 int i,ret;
 ns_get=(void *)shadow_hook_resolve("vns_get_user_ns");
 ns_put=(void *)shadow_hook_resolve("vns_put_user_ns");
 groups_show=(void *)shadow_hook_resolve("vns_proc_setgroups_show");
 instantiate=(void *)shadow_hook_resolve("proc_pident_instantiate");
 if(!ns_get || !ns_put || !groups_show || !instantiate) return -ENOENT;
 for(i=0;i<4;i++) {
  entries[i].name=names[i]; entries[i].len=strlen(names[i]);
  entries[i].mode=S_IFREG|0644; entries[i].fop=&fops[i];
  writers[i]=(void *)shadow_hook_resolve(wn[i]); if(!writers[i]) return -ENOENT;
  if(i<3) { seqs[i]=(void *)shadow_hook_resolve(sn[i]); if(!seqs[i]) return -ENOENT; }
 }
 ret=shadow_hook_install(&hooks[0]); if(ret) return ret;
 ret=shadow_hook_install(&hooks[1]);
 if(ret) { shadow_hook_remove(&hooks[0]); return ret; }
 __module_get(THIS_MODULE);
 pr_info("ds_proc_idmap_compat: native proc PID mapping lookup enabled (boot-lifetime)\n");
 return 0;
}
module_init(compat_init);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Native proc PID id-map dentries for TB323FU Android 6.12");
