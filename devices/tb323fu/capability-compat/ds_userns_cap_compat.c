// Public Domain since creation (AI-generated; no copyright subsists).
#include <linux/module.h>
#include <linux/cred.h>
#include <linux/capability.h>
#include <linux/prctl.h>
#include "shadow_hook.h"

static struct cred *(*real_prepare_creds)(void);
static int (*real_commit_creds)(struct cred *);

static int (*native_prctl)(int, unsigned long, unsigned long,
                           unsigned long, unsigned long);

/* CONFIG_USER_NS=n folds current_user_ns() to init_user_ns inside the
 * native bounding-set drop. Use the credential's actual namespace only
 * for this operation; all other prctl and LSM checks remain native. */
static int __nocfi compat_prctl(int option, unsigned long arg2,
                               unsigned long arg3, unsigned long arg4,
                               unsigned long arg5)
{
 const struct cred *old = current_cred();
 struct cred *new;
 if (option != PR_CAPBSET_DROP || old->user_ns == &init_user_ns)
  return native_prctl(option, arg2, arg3, arg4, arg5);
 if (!ns_capable(old->user_ns, CAP_SETPCAP))
  return -EPERM;
 if (!cap_valid(arg2))
  return -EINVAL;
 new = real_prepare_creds();
 if (!new)
  return -ENOMEM;
 cap_lower(new->cap_bset, arg2);
 return real_commit_creds(new);
}
static const char * const names[] = { "cap_task_prctl", NULL };
static struct shadow_hook hook = SHADOW_HOOK(names, compat_prctl, &native_prctl);
static int __init compat_init(void)
{
 real_prepare_creds = (void *)shadow_hook_resolve("prepare_creds");
 real_commit_creds = (void *)shadow_hook_resolve("commit_creds");
 if (!real_prepare_creds || !real_commit_creds)
  return -ENOENT;
 return shadow_hook_install(&hook);
}
static void __exit compat_exit(void) { shadow_hook_remove(&hook); }
module_init(compat_init);
module_exit(compat_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Preserve namespace-local authorization for capability drops");
