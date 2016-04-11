#include <linux/list.h>
#include <linux/vmalloc.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <asm/string.h>
#include <asm-generic/uaccess.h>
#include <linux/spinlock.h>

#define MAXMOD 200
#define MAX_NAME 20
MODULE_LICENSE("GPL");

struct list_head proc_list;
int nr_procs;
spinlock_t sp_proc;

//ITEM FOR PROC LIST
struct list_item_procs{
	char name[MAX_NAME];
	struct list_head links;
};

struct private_data_proc{
	struct list_head list;
	spinlock_t sp;
	int nr_data_list;
};

struct list_item_lists{
	int data;
	struct list_head links;
};

static struct proc_dir_entry *default_entry;
static struct proc_dir_entry *control_entry;
static struct proc_dir_entry *list_dir = NULL;

static ssize_t proc_read(struct file *f, char __user *buf, size_t len, loff_t *off) {
	int nr_bytes = 0;
	char *buffer, *aux;
	struct list_head *cur_node;
	struct private_data_proc *private;
	struct list_item_lists *item;
	
	buffer = (char *)vmalloc(MAXMOD);
	aux = (char *)vmalloc(sizeof(int)+1);
	
	if ((*off) > 0) // Tell the application that there is nothing left to read 
      		return 0;

	private = PDE_DATA(f->f_inode);
		
	if(private == NULL){
		printk(KERN_INFO "PRIVATE NULL\n");
	} else{
		
		spin_lock(&private->sp);
			if(list_empty(&private->list)){
				sprintf(buffer, "");
			} else {
				list_for_each(cur_node, &private->list){
					item = list_entry(cur_node, struct list_item_lists, links);
					//printk(KERN_INFO "Mostrando %i\n", item->data);
					if(nr_bytes == 0) sprintf(buffer, "%i\n", item->data);
					else{
						sprintf(aux, "%i\n", item->data); 
						strcat(buffer, aux);
					}
					nr_bytes+=sizeof(int)+1;
				}
			}
		spin_unlock(&private->sp);
		
	}

	if (len < nr_bytes)
		return -ENOSPC;
	
	if (copy_to_user(buf, buffer, nr_bytes))
		return -EINVAL;
	
	(*off) += len;
	
	vfree(buffer);
	vfree(aux);
	
	return nr_bytes;
}

static ssize_t proc_write(struct file *f, const char __user *buf, size_t len, loff_t *off) {
	char *buffer, *n;
	int data;
	struct list_item_lists *item_list;
	struct private_data_proc *private;
	struct list_head *cur_list, *aux_list;
	
	buffer = (char *)vmalloc(len);
	
	if ((*off) > 0){ // Tell the application that there is nothing left to read 
		vfree(buffer);
		return 0;
	}
	
	if (copy_from_user( buffer, buf, len )) {
		vfree(buffer);
		return -EFAULT;
	}
	
	n = f->f_path.dentry->d_iname;
	private = (struct private_data_proc *) PDE_DATA(f->f_inode);
	
	if(private == NULL){
		printk(KERN_INFO "PRIVATE NULL\n");
		vfree(buffer);
		return len;
	}
	
	if(sscanf(buffer, "add %i", &data)){
		item_list = (struct list_item_lists *) vmalloc(sizeof(struct list_item_lists));
		item_list->data = data;
		spin_lock(&(private->sp));
			list_add_tail(&item_list->links, &(private->list));
			private->nr_data_list++;
		spin_unlock(&(private->sp));
		printk(KERN_INFO "%i was inserted correctly in %s.\n", item_list->data, n);
	}
	else if(sscanf(buffer, "remove %i", &data)){
		spin_lock(&sp_proc);
			if(list_empty(&proc_list)){
				printk(KERN_INFO "THERE ARE NO /PROC.\n");
				return len;
			} 
			printk(KERN_INFO "All %i values will be deleted from %s.\n", data, n);
			
			//DELETING FROM /PROC THE NUMBRE IN ITS LIST ASSOCIATED
			if(private == NULL){
				printk(KERN_INFO "There are no /proc named %s.\n", n);
			}else if(list_empty(&(private->list)) == 0){
				spin_lock(&(private->sp));
					list_for_each_safe(cur_list, aux_list, &(private->list)){
						item_list = list_entry(cur_list, struct list_item_lists, links);
						if(item_list->data == data){
							list_del(cur_list);
							private->nr_data_list--; 
							//vfree(aux_list);
						}
					}
				spin_unlock(&(private->sp));
			}else printk(KERN_INFO "EMPTY LIST. IMPOSIBLE TO DELETE.\n");
		spin_unlock(&sp_proc);
	}
	else if(strcmp(buffer, "cleanup\n")== 0){
		spin_lock(&sp_proc);
			if(list_empty(&proc_list)){
				printk(KERN_INFO "THERE ARE NO /PROC.\n");
				return len;
			} 
			printk(KERN_INFO "All values will be deleted from %s.\n", n);
			
			//DELETING FROM /PROC THE NUMBRE IN ITS LIST ASSOCIATED
			if(private == NULL){
				printk(KERN_INFO "There are no /proc named %s.\n", n);
			} else if(list_empty(&(private->list)) == 0){
				spin_lock(&(private->sp));
					list_for_each_safe(cur_list, aux_list, &(private->list)){
						item_list = list_entry(cur_list, struct list_item_lists, links);
							list_del(cur_list);
							private->nr_data_list--; 
							//vfree(aux_list);
					}
				spin_unlock(&(private->sp));
			}else printk(KERN_INFO "EMPTY LIST. IMPOSIBLE TO DELETE.\n");
		spin_unlock(&sp_proc);
	}
	else {
		printk(KERN_INFO "Not implemented function.\n");
	}

	vfree(buffer);

	return len;
}

static const struct file_operations default_entry_fops = {
	.read = proc_read,
	.write = proc_write,
};

static ssize_t control_write(struct file *f, const char __user *buf, size_t len, loff_t *off) {
	char *buffer;
	char *data;
	struct list_item_procs *item;
	struct private_data_proc *priv;
	struct list_head *cur, *aux;
	
	buffer = (char *)vmalloc(len);
	
	if ((*off) > 0){ /* Tell the application that there is nothing left to read */
		vfree(buffer);
		return 0;
	}
	
	if (copy_from_user( buffer, buf, len )){  
		vfree(buffer);
		return -EFAULT;
	}

	if(strncmp(buffer, "create ", 7) == 0){
		buffer[len - 1] = '\0';
		data = &buffer[7];
		
		//CONTROLING NOT TO EXCEED THE NUMBER OF MAX PROCS AND DONT NAME THE SAME AS CONTROL.
		if(strcmp("control", data) != 0){
			
			//CHECK THERE'S NO OTHER FILE NAME THE SAME WAY.
			int i = 0;
			list_for_each(cur, &proc_list){
				item = list_entry(cur, struct list_item_procs, links);
				if(item != NULL){
					if(strcmp(item->name, data)==0){
						i = 1;
					}
				}
			}
			
			//IF NOT, JUST CREATE IT
			if(i == 0){
				item = (struct list_item_procs *) vmalloc(sizeof(struct list_item_procs));
				priv = (struct private_data_proc *) vmalloc(sizeof(struct private_data_proc));
				
				//INIT THE PARAMS ON PRIV LIST DATA
				INIT_LIST_HEAD(&priv->list);
				spin_lock_init(&priv->sp);
				priv->nr_data_list = 0;
				
				default_entry = proc_create_data(data, 0666, list_dir, &default_entry_fops, priv);
				if (default_entry == NULL){
					printk(KERN_INFO "Error en %s entry\n", data);
					vfree(item);
					vfree(buffer);
					vfree(priv);
					return -ENOMEM;
				}
				
				//INIT THE PARAMS ON THE STRUCTURE
				strcpy(item->name, data);
				
				//ADDING THE ITEM TO THE LIST OF PROCS
				spin_lock(&sp_proc);
					list_add_tail(&item->links, &proc_list);
					nr_procs++;
				spin_unlock(&sp_proc);
				
				printk(KERN_INFO "Has been created /proc/multilist/%s.\n", data);
			} else {
				printk(KERN_INFO "There's a /proc/multilist/%s already.\n", data);
			}
		}else{
			printk(KERN_INFO "Create control /proc not allowed.\n");
		}
	}
	else if(strncmp(buffer, "delete ", 7) == 0){
		buffer[len - 1] = '\0';
		data = &buffer[7];
		
		if(nr_procs != 0 && strcmp("control", data) != 0){
			int i = 0;
			
			//CHECKING IF THE LIST IS EMPTY. IF NOT, CHECKING IF DATA IS IN THE LIST
			spin_lock(&sp_proc);
				if(list_empty(&proc_list)){
					printk(KERN_INFO "EMPTY LIST. IMPOSIBLE TO DELETE\n");
					spin_unlock(&sp_proc);
					vfree(buffer);
					return len;
				} 
				
				list_for_each_safe(cur, aux, &proc_list){
					item = list_entry(cur, struct list_item_procs, links);
					
					//IF MATCH, DELETE
					if(strcmp(item->name, data) == 0){
						remove_proc_entry(item->name, list_dir);
						list_del(cur);
						//vfree(aux);
						printk(KERN_INFO "Has been deleted /proc/multilist/%s.\n", data);
						nr_procs--;
						i = 1;
					} 
				}
			spin_unlock(&sp_proc);
			
			//IF THERE WERE NO MATCHING...
			if(i == 0) printk(KERN_INFO "There's no /proc named %s to be deleted.\n", data);
		} else {
			printk(KERN_INFO "There's no /proc to be deleted.\n");
		}
	}
	else{
		printk(KERN_INFO "Not implemented function.\n");
	}

	vfree(buffer);
	
	return len;
}


static const struct file_operations control_entry_fops = {
	.write = control_write,
};

int init_modlist_module(void){
	struct list_item_procs *item;
	struct private_data_proc *priv;
	
	//INIT THE LIST OF /PROC AND ITS SPINLOCK
	INIT_LIST_HEAD(&proc_list);
	spin_lock_init(&sp_proc);
    nr_procs = 0; 
	
	//CREATING THE DIR IN /PROC
	list_dir = proc_mkdir("multilist", NULL);
	if (!list_dir) {
        printk(KERN_INFO "Error en la creación de /proc/multilist\n");
        return -ENOMEM;
    }

	item = (struct list_item_procs *) vmalloc(sizeof(struct list_item_procs));
	priv = (struct private_data_proc *) vmalloc(sizeof(struct private_data_proc));
	
	//INIT THE PARAMS ON PRIV LIST DATA
	INIT_LIST_HEAD(&priv->list);
	spin_lock_init(&priv->sp);
	priv->nr_data_list = 0;
	
	//CREATING THE 2 /PROC DEFAULT AND CONTROL. DEFAULT WILL BE NSERTED IN THE PROC_LIST
	default_entry = proc_create_data("default", 0666, list_dir, &default_entry_fops, priv);
	if (default_entry == NULL) {
		remove_proc_entry("multilist", NULL);
		printk(KERN_INFO "Error en default_entry\n");
		return -ENOMEM;
	} 
	
	//INIT THE PARAMS ON THE STRUCTURE
	strcpy(item->name, "default");
	nr_procs++;

	control_entry = proc_create( "control", 0666, list_dir, &control_entry_fops);
	if (control_entry == NULL) {
		remove_proc_entry("default", list_dir);
		remove_proc_entry("multilist", NULL);
		printk(KERN_INFO "Error en control_entry\n");
		return -ENOMEM;
	} 
	
	//ADDING DEFAULT TO THE LIST
	spin_lock(&sp_proc);
		list_add_tail(&item->links, &proc_list);
		nr_procs++;
	spin_unlock(&sp_proc);
	
	printk(KERN_INFO "Modulo iniciado.\n");
	
	return 0;
}


void clean_modlist_module(void){
	struct list_head *cur, *aux; 
	struct list_item_procs *item;  
	
	//DELETING OF THE DATA FROM THE LIST OF /PROCS
	spin_lock(&sp_proc);
			if(list_empty(&proc_list)){
				printk(KERN_INFO "EMPTY /PROC LIST.\n");
			} else {
				list_for_each_safe(cur, aux, &proc_list){
					item = list_entry(cur, struct list_item_procs, links);
					remove_proc_entry(item->name, list_dir);
					list_del(cur);
					//vfree(aux);
					nr_procs--; 
				}
			}
	spin_unlock(&sp_proc);

	//DELETING CONTROL AND THE DIR.
	remove_proc_entry("control", list_dir);
	remove_proc_entry("multilist", NULL);
	
	printk(KERN_INFO "Modulo descargado.\n");
}

module_init( init_modlist_module );
module_exit( clean_modlist_module );






