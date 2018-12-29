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

### DEVICE
```
CPU     :       intel core i7-4790k (4C8T) 4.6GHz
RAM     :       16G
OS      :       ubuntu-16.04.5-desktop-i386(Kernel 4.6.0)
gcc     :       4.8.5
thread  :       posix  
```
### SOLUTION
#### Car Behavior
We solve the problem by assign a thread for each car and solve the problem with the perspective of the car thread.  
1) when a car thread generate, the car will get two attribute, **direction** and **number**. And the number actually is the car's ID and the number of any two car will not be equal.
2) The car will enter a **wait queue** for its direction. To prevent other car operate the wait queue when this car is operating. So we use a **mutex lock** for the wait queue.
3) The car will judge if the number of the car in the head of the wait queue is the same of its number. If **not**, the car will stuck in the wait queue until the queue head number is just its own number. If the car is **in the queue head**, the car tell the output device (e.g. our monitor) it has arrived the cross and can't wait to start to pass the cross. And at the same time the car behind this car will not have the the opportunity to become the queue head and tell the output device it arrives before the former car quit from the queue and give the car behind it a **broadcast** signal.
4) All the car stuck in the queue will receive the broadcast and **re-judge** if they become the head of the queue. Only the new head car will continue and the others will back to the wait status till another car get out of the queue and give a new broadcast. This setting make sure there can only be **one car** enter the **judge and pass status**. That keep the thread safe.
5) Now the head car tell the monitor it arrived just the same as the car in **step 3**.
6) The car will evaluate the status in the cross. According the passing rules, the right-side car has the privilege to ge first. So the current car will judge if there be some car in its right side. So the car thread will check the size of the queue in its right direction. If the size **equal zero** that means at this time the right side has no car and it could get into the cross directly if the cross is empty. If the size is **not equal to zero**, the current car must wait till the right side car pass the cross and send a **first-signal** to it. And the car will use a **flag** to tell the god if it's in the **waiting status**.
7) After get the first signal from the right side. The car then check if the cross is empty. If **not empty**, wait the dude in the cross to send a signal. If **empty**, get into it, tell the others some dude is in the cross and pass through.
8) The current car tell the monitor it is leaving. And the car give the left-side car a **first-sinal** to tell the car now it could start to enter the cross once the cross is empty.
9) Then the car get out of the cross and tell others there is no dude in the cross and give the car which is waiting to enter the cross a **sinal** that "I'm out of the cross!".
10) Then the car quit the queue and the car behind it can check if they are the new head now.  


#### DeadLock Check
We don't let the car to check if the deadlock happens, we let a character who is just like a policeman to check the deadlock status and give its order.  
1) The checkman will judge if the head car in the 4 diretion is in the **waiting** status (In the **step6** the car tell the god whether it is waiting status.
2) If not all the directions are in the waiting stauts, then ignore the remain operation and keep check. If all the directions are in the waiting status, then give a signal to the **NORTH** direction let the head car of the NORTH direction to go first.


#### Remark
* `pthread_cond_wait` is a interesting function, we pass the two parameters to it, the `pthread_cond_t` and the `pthread_mutex_t`. When the function is invoked, we let the thread to wait until the cond is satisfied, and meanwhile we find the mutex passed in, and **unlock** the mutex! So when this thread is waiting for the conditon, other thread could do something in the critcal section. (It's just like if we don't pull, we hand over my pit so others could use.) And when the condition is satisfied, we regain the possesion of the mutex(that is lock the dirMut again) The process just like that:
  ```
  invoke->unlock the mutex->wait->conditon satisfied->lock again->continue
  ```
  And we usually usr the `pthread_cond_t` with the `while` loop and `pthread_mutex_t`. Check the example below:
  ```cpp
  //thread A
  pthread_mutex_lock(mutex);
  while(!ConditionStatisfied())pthread_cond_wait(&conditon,&mutex);
  pthread_mutex_unlock(mutex);

  //thread B
  pthread_mutex_lock(mutex);
  signal(&condition);
  pthread_mutex_unlock(mutex);
  ```
  `thread A` will wait the conditon to be satified and release the control of the critical section at the same time. And `thread B` could regain the control of critical section and then signal the `thread A` the conditon is satisfied. Then `thread B` hand out the critical section. After the `thread A` is awake the `thread A` will try to regain the control of the critical section so it will get the mutex after the `thread B` release the mutex. And before it continue, `thread A` should also check if the condition in the `while` loop is still statisfied. That prevent the situation that the `thread A` is signaled but the condition in the `while` loop has lost it power. The internal in the `thread A` could be describ like this:
  ```cpp
  unlock(mutex);
  wait();
  lock(mutex);
  ```
  And the `pthread_cond_wait()` merges them as a atomic operation.
* Some subtle things may happen when the car in the same direction is wait in the queue.
