# Assignment 3

- Student ID: 20160233
- Your Name: ParkNaHyeon
- Submission date and time: 2020.11.22 around 9pm

## Ethics Oath
I pledge that I have followed all the ethical rules required by this course (e.g., not browsing the code from a disallowed source, not sharing my own code with others, so on) while carrying out this assignment, and that I will not distribute my code to anyone related to this course even after submission of this assignment. I will assume full responsibility if any violation is found later.

Name: ParkNaHyeon
Date: 2020.11.22

## Design Report (**IMPORTANT**)

1. Explanation about thread pool, task queue, and task allocation (10 points)
    - design of thread pool, task queue
    thread pool has 10 threads.
    My program has 1 task queue. Threads of thread pool share one task queue.
    Each task queue entry has client socket fd(cli_sockfd) number in it.
    
    - task allocation policy
    At first, random thread of thread pool gets the task when it gets signal from accept callback function.
    Then, other threads get the mutex_lock when pthread_cond_wait signal is satisfied. It then goes in to the loop to find the task from task queue.
    When a thread finds non-zero task_queue entry, it saves the client_sockfd on its own and dequeues(makes the entry to zero) and goes on to its routine.
    When a thread finishes its routine(parse client request, save key-value pair to redis server, get response from redis server, send response to client), it waits for signal from accept callback function via pthread_cond_wait. When it receives the signal, it goes through the task_queue to find non-empty entry and goes through its routine again.

2. Explanation about Managing Redis connection, connection allocation (10 points)
    - Redis connection allocation policy
    Each thread makes connection with redis server. It makes four connection with redis server but mine just uses one connection per thread(total at most 10) due to too many open fds(program crashes then,,,). And since my program design order is "a thread parses the request from client(at one bufferevent receiving) -> makes key_buffer(that contains key buffer) sends it to redis server -> makes value buffer and send it to redis server -> receive more from client... so and on). According to my design, I couldn't think of better way for redis connection. Because iterating through connection is more inefficient since it opens too many socket decriptors while my design it impossible to concurrently send request to redis server since it does not save the whole request from client but sends the key&value right after it receives it. So I thought, in the scope of my design order, using one redis connection per one thread is better option.
    
Please describe your system with detail.
First, in "main function", it makes the event base for accept(in specific, accepting client request). My webserver's sockfd is connected with that event. (libevent). Then, when event is evoked, it executes "accept callback(accept_cb)"function. Also, it makes ten threads of thread pool. Each thread of thread pool has its own bufferevent base. 
In accept callback function, it accepts the client request and put the client sockfd number into empty task_queue entry and send signal to threads waiting for the signal.
In "thread routine(thread_pool_func)", threads wait for accept callback function to send signal when task_queue has task to do. When a thread receives a signal, it deques the cli_sockfd number from task_queue and make bufferevent with that socket fd(cli_sockfd). Then, it dispatches(event_base_dispatch) through its own base. 
When "read callback of bufferevent(cli_readcb)" is invoked, thread parses the request from client and makes bufferevent(which sets redis's sockfd) on its own base. Then, it sends the request to redis server at one bufferevent_read. 
When "redis_callback function(redis_readcb)" is invoked. It checks whether it got right number of "+OK" from redis server, then it sends the response back to client.
This is a whole sequence of task that each thread of thread pool goes through. Since each thread has its owm bufferevent base and listening thread has its own libevent base, each thread(total 11) goes through its own event loop and does its job.
This is overall sequence of my program. 

# Misc
Describe here whatever help (if any) you received from others while doing the assignment.
No.

How difficult was the assignment? (1-5 scale) 4.5
*****In test/test.py, in robust_set_1, the script tells that the last value(of key) is None. However, when I check what I sent and the redis response, it correctly does the job. So, I think I did it right... Please check this problem whether it's a problem related to test python script. Thank you.

How long did you take completing? (in hours) about 50 hours
