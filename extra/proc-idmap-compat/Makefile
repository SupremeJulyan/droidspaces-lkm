# Build against the exact deployed lkm4ctr source and Module.symvers.
ifndef LKM4CTR_ROOT
$(error Set LKM4CTR_ROOT to the matching lkm4ctr source directory)
endif
obj-m += ds_proc_idmap_compat.o
ccflags-y += -I$(srctree)/fs -I$(LKM4CTR_ROOT)/glue
