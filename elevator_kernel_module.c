
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/linkage.h>
#include <linux/list.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/mutex.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jack & Austin");
MODULE_DESCRIPTION("Operates an Elevator");
MODULE_VERSION("0.01");

#define BUF_LEN 500

//Stubs used for the system calls
extern long (*STUB_start_elevator)(void);
extern long (*STUB_issue_request)(int,int,int);
extern long (*STUB_stop_elevator)(void);

//Variables used for the proc file and prints 
struct proc_dir_entry* proc_entry;
char msg[BUF_LEN];
int procfs_buf_len;

//struct that contains the data for each passender
typedef struct Passenger {
	int type; //0-human 1-zombie
	int start;
	int dest;
} Passenger;

//struct used for the list, list of Passenger structs
typedef struct item {
	struct list_head list;
	Passenger pass;
} Item;
//declaration of array of lists, each floor is an array element
struct list_head floor_pass[10];

struct thread_parameter {
	int id;
	int cnt;
	struct task_struct *kthread;
};
//thread and lock declarations
struct thread_parameter thread1;
struct mutex my_mutex;

//global variables used throughout the program
int elevator_state; //0-offline 1-idle 2-loading 3-up 4-down
int elevator_status; //0-human 1-zombie
int current_floor;
int num_passengers;
int num_passengers_waiting;
int num_passengers_serviced;
Passenger passengerArr[10];
int floor_pass_waiting[10];
int powering_off;

static ssize_t procfile_read(struct file* file, char * ubuf, size_t count, loff_t *ppos) {

	char str_elevator_state[16];
	char str_elevator_status[16];
	char str_floor[25];

	int i;
	struct list_head *temp;
	Item *item;

	mutex_lock(&my_mutex);	
	//creates the message to be printed out to the user, usr,
	if(elevator_state == 0)
		strcpy(str_elevator_state, "OFFLINE");
	else if(elevator_state == 1)
		strcpy(str_elevator_state, "IDLE");
	else if(elevator_state == 2)
		strcpy(str_elevator_state, "LOADING");
	else if(elevator_state == 3)
		strcpy(str_elevator_state, "UP");
	else if(elevator_state == 4)
		strcpy(str_elevator_state, "DOWN");

	if(elevator_status == 0)
		strcpy(str_elevator_status, "Not-Infected");
	else if(elevator_status == 1)
		strcpy(str_elevator_status, "Infected");

	sprintf(msg, "Elevator state: %s\nElevator status: %s\nCurrent floor: %i\nNumber of passengers: %i\nNumber of passengers waiting: %i\nNumber passengers serviced: %i\n", str_elevator_state, str_elevator_status, current_floor, num_passengers, num_passengers_waiting, num_passengers_serviced);

	//loop prints out each floor and the people on it
	for(i = 9; i>=0;i--)
	{
		strcat(msg,"[");
		if(current_floor == i+1) 
			sprintf(str_floor,"*] Floor ");
		else
			sprintf(str_floor," ] Floor ");
		strcat(msg,str_floor);
		if(i != 9)
			strcat(msg," ");
		sprintf(str_floor, "%i:   %i  ",i+1,floor_pass_waiting[i]);
		strcat(msg,str_floor);

		list_for_each(temp, &floor_pass[i]) {
			item = list_entry(temp,Item,list);
			if(item->pass.type == 0)
				strcat(msg, " |");
			else
				strcat(msg, " X");
		}
		strcat(msg,"\n");
	}
	mutex_unlock(&my_mutex);
		

	procfs_buf_len = strlen(msg);
	
	if(*ppos > 0 || count < procfs_buf_len) {
		return 0;
	}
	if(copy_to_user(ubuf, msg, procfs_buf_len)) {
		return -EFAULT;
	}
	*ppos = procfs_buf_len;

	return procfs_buf_len;
}
//redirects to the print out function
static struct file_operations myops = 
{
	.owner = THIS_MODULE,
	.read = procfile_read,
};


//starts the elevator and initialized some of the elevator values 
long start_elevator(void) {
	int yeet;
        printk(KERN_WARNING "Starting Elevator\n");
	mutex_lock(&my_mutex);
	
	if(elevator_state != 0)
		yeet = 1;
	else {
		elevator_state = 1;
		yeet = 0;
		current_floor = 1;
		num_passengers = 0;
	}
	mutex_unlock(&my_mutex);
        return yeet;
}
long issue_request(int start, int dest, int type) {
	Item *item;
	int yeet;
	//takes the info from the file and adds that person to the wait list for that given floor
        printk(KERN_WARNING "Issuing Request Start: %i Stop: %i Type: %i\n",start,dest,type);
	if(start < 1 || start > 10 || dest < 1 || dest > 10 || type < 0 || type > 1)
		yeet = 1;
	else
		yeet = 0;
	mutex_lock(&my_mutex);
	item = kmalloc(sizeof(Item), __GFP_RECLAIM);
	item->pass.start = start;
	item->pass.dest = dest;
	item->pass.type = type;
	list_add_tail(&item->list, &floor_pass[start-1]);
	floor_pass_waiting[start-1]++;
	num_passengers_waiting++;
	mutex_unlock(&my_mutex);
        return yeet;
}
long stop_elevator(void) {
	int yeet;
	//prompts the elevator to stop once its done dropping its people off
	mutex_lock(&my_mutex);
        printk(KERN_WARNING "Stopping Elevator\n");
	if(elevator_state == 0)
		yeet = 1;
	else {	
		powering_off = 1;
        	yeet = 0;
	}
	mutex_unlock(&my_mutex);
	return yeet;
}
//main function that deals with the workings and running of the elevator
int elevator_run(void *data) {
	struct thread_parameter *parm = data;
	int i, load, end, up, down;
	Item *item;
	//main loop for the elevator that runs until the exit function ends the thread
	while (!kthread_should_stop()) {
		up = down = 0;
		mutex_lock(&my_mutex);
		load = end = 0;
		parm->cnt++;
		//stops elevator once all the people have gotten off
		if(powering_off == 1 && num_passengers == 0) {
			powering_off = 0;
			elevator_state = 0;
		}
		//designates where people are waiting and where the people's destinations are
		//redirects elevator if it is going to somewhere empty
		for(i = 0;i<10;i++) {
			if(passengerArr[i].dest < current_floor && passengerArr[i].dest != 0)
				down = 1;
			if(passengerArr[i].dest > current_floor && passengerArr[i].dest != 0)
				up = 1;
			if(floor_pass_waiting[i] > 0 && i+1 < current_floor && powering_off == 0)
				down = 1;
			if(floor_pass_waiting[i] > 0 && i+1 > current_floor && powering_off == 0)
				up = 1;
		}


		//tells the elevator if it should be going up, down or idling
		if(up == 1 && down == 0 && elevator_state != 0)
			elevator_state = 3;
		else if(up == 0 && down == 1 && elevator_state != 0)
			elevator_state = 4;
		else if(current_floor == 1 && elevator_state != 0)
			elevator_state = 3;
		else if(current_floor == 10 && elevator_state != 0)
			elevator_state = 4;
		else if(num_passengers_waiting == 0 && num_passengers == 0 && elevator_state != 0)
			elevator_state = 1;
		else if(elevator_state == 1 && num_passengers_waiting > 0 && current_floor <=5)
			elevator_state = 3;
		else if(elevator_state == 1 && num_passengers_waiting > 0 && current_floor >=6)
			elevator_state = 4;	
		//checks and unloads people on the elevator and adjusts the correct member data
		for(i = 0; i<10;i++) {
			if(passengerArr[i].dest == current_floor) {
				num_passengers_serviced++;
				num_passengers--;
				passengerArr[i].dest = 0;
				passengerArr[i].start = 0;
				passengerArr[i].type = 0;
				if(num_passengers == 0)
					elevator_status = 0;
				load = 1;
			}
		}
		//checks to people on the currect floor if they should be picked up
		//checks a variety of different conditions
		if(floor_pass_waiting[current_floor-1] != 0 && elevator_state != 0) {
			//while loop runs to so it can pick up more than one person
			while(end == 0 && floor_pass_waiting[current_floor-1] > 0) {
				item = list_first_entry(&floor_pass[current_floor-1], Item, list);
				if((((elevator_state == 3 && (current_floor < item->pass.dest)) || (elevator_state == 4 && (current_floor > item->pass.dest))) && (elevator_status == 0 || item->pass.type == 1)) && num_passengers < 10) {
					//checks for the first available spot in the elevator and loads the person in
					for(i=0;i<10;i++) {
						if(passengerArr[i].start == 0) {
							passengerArr[i].start = item->pass.start;
							passengerArr[i].dest = item->pass.dest;
							passengerArr[i].type = item->pass.type;
							num_passengers++;
							floor_pass_waiting[current_floor-1]--;
							num_passengers_waiting--;
							load = 1;
							printk(KERN_WARNING "Loaded person start: %i dest: %i type: %i\n",item->pass.start,item->pass.dest,item->pass.type);
							break;
						}
					}
					//changed the elevator and passenger state if they are infected
					if(item->pass.type == 1) {
						printk("Elevator infected\n");
						elevator_status = 1;
						for(i=0;i<10;i++)
							passengerArr[i].type = 1;
					}
					//deleted item from list
					list_del(&item->list);
					kfree(item);
				}
				else
					end = 1;	
			}
		}
		//sleeps for a second if the elevator loads or unloads
		mutex_unlock(&my_mutex);
		if(load == 1)
			ssleep(1);				

		mutex_lock(&my_mutex);
		//moved the elevator once all the passengers are loaded and unloaded
		if(elevator_state == 3 && (num_passengers_waiting != 0 || num_passengers != 0))
			current_floor++;
		else if(elevator_state == 4 && (num_passengers_waiting != 0 || num_passengers != 0))
			current_floor--; 
		else if(elevator_state == 0 && num_passengers > 0 && down == 1)
			current_floor--;
		else if(elevator_state == 0 && num_passengers > 0 && up == 1)
			current_floor++;
		mutex_unlock(&my_mutex);
		//sleeps the elevator for 2 seconds while it moves floors
		ssleep(2);
	}
	return 0;
}
//function that calls the use of the thread for the main elevator function
void thread_init_parameter(struct thread_parameter *parm) {
	static int id = 1;

	parm->id = id++;
	parm->cnt = 0;
	parm->kthread = kthread_run(elevator_run, parm, "Elevator Thread %d", parm->id);
}


//initiazation function called when loaded into kernel. also loads elevator function into a separate thread
//initializes the lists, proc, mutex, and thread.
static int elevator_init(void) {
	STUB_start_elevator = start_elevator;
	STUB_issue_request = issue_request;
	STUB_stop_elevator = stop_elevator;
	proc_entry=proc_create("elevator",0777,NULL,&myops);

	INIT_LIST_HEAD(&floor_pass[0]);
	INIT_LIST_HEAD(&floor_pass[1]);
	INIT_LIST_HEAD(&floor_pass[2]);
	INIT_LIST_HEAD(&floor_pass[3]);
	INIT_LIST_HEAD(&floor_pass[4]);
	INIT_LIST_HEAD(&floor_pass[5]);
	INIT_LIST_HEAD(&floor_pass[6]);
	INIT_LIST_HEAD(&floor_pass[7]);
	INIT_LIST_HEAD(&floor_pass[8]);
	INIT_LIST_HEAD(&floor_pass[9]);

	elevator_state = 0;
	elevator_status = 0;
	current_floor = 1;
	num_passengers = 0;
	num_passengers_waiting = 0;
	num_passengers_serviced = 0;
	powering_off = 0;

	thread_init_parameter(&thread1);
	mutex_init(&my_mutex);	

	return 0;
}
module_init (elevator_init);
//function called when elevator unloaded from kernel. Sets syscall stubs to null.
//deallocates the mutex and thread. also removes all remaining nodes from lists
static void elevator_exit(void) {
	int i;
	struct list_head *pos, *q;
	Item *temp;
	STUB_start_elevator = NULL;
        STUB_issue_request = NULL;
        STUB_stop_elevator = NULL;

	kthread_stop(thread1.kthread);
	mutex_destroy(&my_mutex);	

	proc_remove(proc_entry);
	for(i = 0; i<10;i++) {
		list_for_each_safe(pos, q, &floor_pass[i]) {
			temp = list_entry(pos,Item,list);
			list_del(pos);
			kfree(temp);
			printk(KERN_WARNING "Deleted Item start: %i dest: %i\n",temp->pass.start, temp->pass.dest);
		}
	}	
}
module_exit (elevator_exit);
