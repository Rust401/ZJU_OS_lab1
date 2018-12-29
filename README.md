# Operating System(2018-2019Fall&Winter)LAB1

```
Project Name:   Sychronous Mutual Exclusion and Linux Kernel Module
Student Name:   Hu.Zhaodong
Student ID  :   21714069
Major       :   Environmental Engineering
Email       :   zhaodonghu94@zju.edu.cn
phone       :   15700080428
Date        :   2018.9-2018.12
```

## TARGET
* Learn the **system call** of linux and use the **pthread** library to programing.
* Fully understand the importance of **Atomic operation** to **shared memory**.
* Further understand and master the concept of **process** and **thread** and their **sychronous** and **mutual exclusion**.
* Learn to program use **multi-thread** and handle the **sychronous** and **mutual exclusion** problem.
* Learn the mechanism of **module** in Linux and master how to write a linux **module**.
* Futher understanding of **process** in linux by traversing processes in linux system.

## CONTENT1
There are two roads and two lanes for each road and they intersect with each other. Assume the car can only move forward and can not turn or move backword. If when 4 car from 4 direction arrive the cross simultaneously, just like Fig(a). Then they keep move forward and have to stop with every dude entangled with each other. It's a deadlock problem. From the concept of resources allocation of OS, if both 4 guys want to pass through the cross, their need for resources is just like this:
* The car move to north need a,b.
* The car move to west need b,c.
* The car move to south need c,a.
* The car move to east need d,a.  
Fig1(https://github.com/Rust401/ZJU_OS_lab1/blob/master/Synchronous.png?raw=true) 


We need to handle the sychronous problem and prevent the hungry and deadlock of the car when they pass the across. In our system, there are cars from different direction pass through the across. The cars in same direction line up in turn to through the intersection. According to the traffic rule, the car from **right** side get the privilige of go first. (e.g. In Fig(1), assume there is car1, car2 and car3, the order of will be **car1->car2->car3**).  
The general rule list below:
* Cars from same direction pass the cross in order and drive right when they arrive the cross.
* When cars from different direction arrive the cross simultineosly, they car in the **right** side go first unless this guy get a signal of immediate access.
* No **deadlock**.
* No **starving**.
* Not any car thread can control other cars, says, no **single point scheduling**.
* No **AND semaphore**, says the car can not request two road resources simultaneously (e.g. The south car can not judge if a or b are free simultaneously).

## DEVICE
```
CPU     :       intel core i7-4790k (4C8T) 4.6GHz
RAM     :       16G
OS      :       ubuntu-16.04.5-desktop-i386(Kernel 4.6.0)
gcc     :       4.8.5
thread  :       posix  
```
## SOLUTION
We solve the problem by assign a thread for each car and solve the problem with the perspective of the car thread.  
1) when a car thread generate, the car will get two attribute, **direction** and **number**. And the number actually is the car's ID and the number of any two car will not be equal.
2) The car will enter a **wait queue** for its direction. To prevent other car operate the wait queue when this car is operating. So we use a **mutex lock** for the wait queue.
3) The car will judge if the number of the car in the head of the wait queue is the same of its number. If **not**, the car will stuck in the wait queue until the queue head number is just its own number. If the car is **in the queue head**, the car tell the output device (e.g. our monitor) it has arrived the cross and can't wait to start a new trip. And at the same time the car behind this car will not have the the opportunity to become the queue head and tell the output device it arrive.(There is a issur to be solved!!!)
4) After this car arrived the cross, it still in the queue head, but now this dude have to judge the situation in the cross. So this car lock the direction mutex.
5) The car should judge whether the car in the same direction before it is still in the cross (To simulate more truly, the car should take some time to pass the cross). If there is a car, it's not the suit time te ready to judge. The car will wait a signal to wait the former car in the same direction get out of the cross.
6) 
