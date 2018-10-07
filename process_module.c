#define MODULE
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/sched.h>
#include<linux/list.h>

int init_module(void)
{
	int processCount=0;
	int processCountRunning=0;
	int processCountInterruptible=0;
	int processCountUninterruptible=0;
	int processCountStopped=0;
	int processCountTraced=0;
	int processCountZombie=0;
	int processCountDead=0;
	int processCountUnknow=0;
	long processState;
	long processExitState;
	
	struct task_struct *p=&init_task;


	//start marking
	printk("@@@ModuleFunctionStart\n");
	//printk("<1>id\t\tname\t\tparent id\t\tparent name\t\tstate\n");
	for(p=&init_task;(p=next_task(p))!=&init_task;)
	{
		printk("@PID:%d\n",p->pid);
		printk("@NAME:%s\n",p->comm);
		printk("@FATHER_PID:%d\n",p->parent->pid);
		printk("@FATHER_NAME:%s\n",p->parent->comm);

		++processCount;

		processState=p->state;
		processExitState=p->exit_state;

		if(processExitState==EXIT_ZOMBIE)++processCountZombie;
		else if(processExitState==EXIT_DEAD)++processCountDead;
		//else break;

		if(processExitState)
		{
			printk("@STATE:%ld\n",processExitState);
			printk("-------------------------------------------------------------------------\n");
			continue;
		}

		if(processState==TASK_RUNNING)++processCountRunning;
		else if(processState==TASK_INTERRUPTIBLE)++processCountInterruptible;
		else if(processState==TASK_UNINTERRUPTIBLE)++processCountUninterruptible;
		else if(processState==TASK_STOPPED)++processCountStopped;
		else if(processState==TASK_TRACED)++processCountTraced;
		else ++processCountUnknow;

		printk("@STATE:%ld\n",processState);
		printk("-------------------------------------------------------------------------\n");
	}

	printk("@TASK_TOTAL           = %d\n",processCount);
	printk("@TASK_RUNNING         = %d\n",processCountRunning);
	printk("@TASK_INTERRUPTIBLE   = %d\n",processCountInterruptible);
	printk("@TASK_UNINTERRUPTIBLE = %d\n",processCountUninterruptible);
	printk("@TASK_STOPPED         = %d\n",processCountStopped);
	printk("@TASK_TRACED          = %d\n",processCountTraced);
	printk("@EXIT_ZOMBIE          = %d\n",processCountZombie);
	printk("@EXIT_DEAD            = %d\n",processCountDead);
	printk("@UNKNOW               = %d\n",processCountUnknow);

	printk("!!!ModuleFunctionFinished\n");

	return 0;
}

void cleanup_module(void)
{
	printk("@Byb~Dude!\n");
}

MODULE_LICENSE("GPL");


		

