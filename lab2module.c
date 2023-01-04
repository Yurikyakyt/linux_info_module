#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/debugfs.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/pid.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/dcache.h>
#include <asm/io.h>

#define BUFFER_SIZE 1024

MODULE_LICENSE("Dual BSD/GPL");

static struct mutex lock;
static struct dentry *debug_dir;
static struct dentry *debug_file;
static struct task_struct* task = NULL;
static struct inode* inode;
static void print_dentry_struct(struct seq_file *file, struct task_struct *task);
static void print_inode(struct seq_file *file, struct task_struct *task);
static int print_struct(struct seq_file *file, void *data);

static ssize_t write_function(struct file *file, const char __user *buffer, size_t length, loff_t *ptr_offset) {
  mutex_lock(&lock);
  char user_data[BUFFER_SIZE];
  unsigned long pid;
  copy_from_user(user_data, buffer, length);
  sscanf(user_data, "pid: %ld", &pid);
  task = get_pid_task(find_get_pid(pid),PIDTYPE_PID);
  single_open(file, print_struct, NULL);
  mutex_unlock(&lock);
  return strlen(user_data);
}

static struct file_operations fops = {
  .read = seq_read,
  .write = write_function,
};

static int __init mod_init(void) {
  mutex_init(&lock);
  debug_dir = debugfs_create_dir("lab2", NULL);
  debug_file = debugfs_create_file("lab2file", 0777, debug_dir, NULL, &fops);
  return 0;
}

static void __exit mod_exit(void) {
  debugfs_remove_recursive(debug_dir);
}


static int print_struct(struct seq_file *file, void *data) {
  print_dentry_struct(file, task);
  print_inode(file, task);
  return 0;
}

static void print_time(struct seq_file *file,unsigned long time, int mode){
  unsigned long t = time;
  unsigned int year = 1970;
  unsigned int mounth;
  unsigned int day;
  unsigned int hour;
  unsigned int minutes;
  unsigned int seconds;
  unsigned int vgo;
  unsigned int numberday;
  while(t>=31536000){
    if (year%4!=0||(year%100==0 && year%400!=0)){
      t=t-31536000;
      year++;
    } else {
      t=t-31622400;
      year++;
    }
  }
  if (year%4!=0||(year%100==0 && year%400!=0)){
    vgo=0;
  } else {
    vgo=1;
  }
  if (t%86400==0){
  numberday=t/86400;
  t=t-numberday*86400;
  } else {
  numberday=t/86400+1;
  t=t-(numberday-1)*86400;
  } 
  if (numberday<=31){ 
    day=numberday;
    mounth=1;
  }
  if (numberday>31 && numberday<=59+vgo){
    day=numberday-31-vgo;
    mounth=2;
  }
  if (numberday>59+vgo && numberday<=90+vgo){
    day=numberday-59-vgo;
    mounth=3;
  }
  if (numberday>90+vgo && numberday<=120+vgo){
    day=numberday-90-vgo;
    mounth=4;
  }
  if (numberday>120+vgo && numberday<=151+vgo){
    day=numberday-120-vgo;
    mounth=5;
  }
  if (numberday>151+vgo && numberday<=181+vgo){
    day=numberday-151-vgo;
    mounth=6;
  }
  if (numberday>181+vgo && numberday<=212+vgo){
    day=numberday-181-vgo;
    mounth=7;
  }
  if (numberday>212+vgo && numberday<=243+vgo){
    day=numberday-212-vgo;
    mounth=8;
  }
  if (numberday>243+vgo && numberday<=273+vgo){
    day=numberday-243-vgo;
    mounth=9;
  }
  if (numberday>273+vgo && numberday<=304+vgo){
    day=numberday-273-vgo;
    mounth=10;
  }
  if (numberday>304+vgo && numberday<=334+vgo){
    day=numberday-304-vgo;
    mounth=11;
  }
  if (numberday>334+vgo && numberday<=365+vgo){
    day=numberday-334-vgo;
    mounth=12;
  }
  hour=t/3600;
  t=t-hour*3600;
  minutes = t/60;
  t=t-minutes*60;
  seconds=t; 
  if (mode==1){
    seq_printf(file,"\tlast opening time = %lu:%lu:%lu  %lu:%lu:%lu\n",year,mounth,day,hour,minutes,seconds);
  } else {
    seq_printf(file,"\trecent changes time = %lu:%lu:%lu  %lu:%lu:%lu\n",year,mounth,day,hour,minutes,seconds);
  }
}

static void print_inode(struct seq_file *file, struct task_struct *task) {
  inode = task->mm->mmap->vm_file->f_inode;
  seq_printf(file,"\ninode {\n");	
  seq_printf(file,"\tinode number =%lu\n",inode->i_ino);
  seq_printf(file,"\tcounter links =%lu\n",inode->i_count);
  seq_printf(file,"\tcount hard link =%lu\n",inode->i_nlink);
  seq_printf(file,"\tsize file(byte) =%lu\n",inode->i_size);
  print_time(file,inode->i_atime.tv_sec,1);
  print_time(file,inode->i_ctime.tv_sec,0);
  seq_printf(file,"}\n");	
}

static void print_dentry_struct(struct seq_file *file, struct task_struct *task) {
  if (task == NULL) {
    seq_printf(file, "Can't find dentry_struct with this pid\n");
  } else  {
    if (task->mm ==0){
	seq_printf(file, "Can't find dentry_truct with this pid\n");
    }else{
        seq_printf(file, "dentry structure: {\n");
	seq_printf(file, "  count: %u,\n", task->mm->mmap->vm_file->f_path.dentry->d_lockref.count);
	seq_printf(file, "  d_time: %u,\n", task->mm->mmap->vm_file->f_path.dentry->d_time);
	seq_printf(file, "  inode_number: %u,\n", task->mm->mmap->vm_file->f_path.dentry->d_inode->i_ino);
	seq_printf(file, "  unhashed: %s,\n", d_unhashed(task->mm->mmap->vm_file->f_path.dentry) ? "true" : "false");
	seq_printf(file, "  unlinked: %s,\n", d_unlinked(task->mm->mmap->vm_file->f_path.dentry) ? "true" : "false");
	seq_printf(file, "}\n");
     }
    } 	  
}

module_init(mod_init);
module_exit(mod_exit);
