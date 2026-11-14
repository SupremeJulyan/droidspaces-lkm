// Public Domain since creation (AI-generated; no copyright subsists).
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/pidfd.h>
#include <linux/ptrace.h>
struct probe_data { unsigned int cmd; };
static int enter(struct kretprobe_instance *ri, struct pt_regs *regs)
{
 ((struct probe_data *)ri->data)->cmd = regs->regs[1];
 return 0;
}
static int leave(struct kretprobe_instance *ri, struct pt_regs *regs)
{
 unsigned int cmd = ((struct probe_data *)ri->data)->cmd;
 if ((long)regs_return_value(regs) == -EOPNOTSUPP &&
     (cmd == PIDFD_GET_USER_NAMESPACE || cmd == PIDFD_GET_IPC_NAMESPACE ||
      cmd == PIDFD_GET_PID_NAMESPACE || cmd == PIDFD_GET_PID_FOR_CHILDREN_NAMESPACE))
  regs_set_return_value(regs, -ENOTTY);
 return 0;
}
static struct kretprobe probe = {
 .kp.symbol_name = "pidfd_ioctl", .entry_handler = enter,
 .handler = leave, .data_size = sizeof(struct probe_data), .maxactive = 128,
};
static int __init compat_init(void) { return register_kretprobe(&probe); }
static void __exit compat_exit(void) { unregister_kretprobe(&probe); }
module_init(compat_init);
module_exit(compat_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Allow userspace procfs fallback for module-provided namespaces");
