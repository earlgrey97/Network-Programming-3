#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>
#include <fcntl.h>

#define MAXBUFLEN 65535
#define BACKLOG 100

//--------------------------------------------------------------
// functions
// order: accept_cb > thread_pool_func > execute_th_routine > ev_redis_callback
//--------------------------------------------------------------
void ev_redis_callback(int sock, short which, void* data);
void execute_th_routine(int cli_sockfd);
void* thread_pool_func(void* args);
void accept_cb(int sock, short which, void* args);
void cli_readcb(struct bufferevent* bev, void* ptr);
void cli_writecb(struct bufferevent* bev, void* ptr);
void cli_eventcb(struct bufferevent* bev, short events, void* ptr);
void redis_readcb(struct bufferevent* bev, void* ptr);
void redis_writecb(struct bufferevent* bev, void* ptr);
void redis_eventcb(struct bufferevent* bev, short events, void* ptr);
int how_many(char redis_input[]);
//--------------------------------------------------------------
// structs and global variables
//--------------------------------------------------------------
// thread pool is global
pthread_t thread_pool[10];

// task queue is global
int task_queue[10];

int accept_cnt = 0;//

// event base & bufferevents
struct event_base* base[10];

struct event_base* accept_base;

int mutex_flag = 0;//
// mutex for threads
pthread_mutex_t mutex;

// thread condition to enter thread_pool_func
pthread_cond_t cond;

// argv2, argv3
char* argv2;
char* argv3;

struct accept_ARGS{
	int mysockfd;
	struct sockaddr_in cli_addr;
};

struct th_ARGS{
	int th_id; // zero to nine
};

struct cli_buf_ARGS{
	struct bufferevent* cli_bev;
	evutil_socket_t cli_sockfd;
};

struct redis_buf_ARGS{
	struct bufferevent* cli_bev;
	struct bufferevent* redis_bev;
	struct event_base* my_base;
	evutil_socket_t ser_sockfd;
	evutil_socket_t cli_sockfd;
	int error_flag;
	int req_flag;
	int check_cnt;
	int done_flag;
};

struct ev_redis_args{
	//int mysock_fd;
	evutil_socket_t ser_sockfd;
	evutil_socket_t cli_sockfd;
	int error_flag;
	int req_flag;
	//struct Multi_arg* multi_ptr;
	//struct event_base* one_base;
};
//--------------------------------------------------------------
//--------------------------------------------------------------


//--------------------------------------------------------------
// function to write onto client buffer
//--------------------------------------------------------------
void cli_writecb(struct bufferevent* bev, void* ptr){
//--------------------------------------------------------------
	;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
void cli_eventcb(struct bufferevent* bev, short events, void* ptr){
//--------------------------------------------------------------
	;
}

//--------------------------------------------------------------
// function to write onto redis buffer
//--------------------------------------------------------------
void redis_writecb(struct bufferevent* bev, void* ptr){
//--------------------------------------------------------------
	;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
void redis_eventcb(struct bufferevent* bev, short events, void* ptr){
//--------------------------------------------------------------
	;
}
//--------------------------------------------------------------
int how_many(char redis_input[]){
//--------------------------------------------------------------
	char* check_ptr;
	int cnt = 0;
	
	check_ptr = redis_input;
	while((*check_ptr)!='\0'){
		if((*check_ptr)=='O') cnt++;
		check_ptr++;
	}
	return cnt;
}
//--------------------------------------------------------------
// executed when redis event is met
//--------------------------------------------------------------
void redis_readcb(struct bufferevent* bev, void* ptr){
//--------------------------------------------------------------
	struct redis_buf_ARGS* args_ptr = (struct redis_buf_ARGS*)ptr;
	//- struct ev_redis_args* args_ptr = (struct ev_redis_args*)data;
	int ser_sockfd_1 = args_ptr->ser_sockfd;
       	int cli_sockfd = args_ptr->cli_sockfd;
	int error_flag = args_ptr->error_flag;
	int req_flag = args_ptr->req_flag;
	//int my_sockfd = args_ptr->mysock_fd;
	//struct event_base* base = args_ptr->one_base;
	//struct Multi_arg* multiple_arg = args_ptr->multi_ptr;
	//struct event* ev_accept;
	//---------------------------------------------------------
	char redis_reply[MAXBUFLEN];
	char final_redis_reply[MAXBUFLEN];
	int check_cnt = 0;
	//---------------------------------------------------------
	char* found_str;
	char reply_to_cli[MAXBUFLEN];
	char value_buf[MAXBUFLEN];
	char* len_buf = (char*)malloc(20);
	int cnt = 0;
	int value_len;
	//---------------------------------------------------------
	struct bufferevent* cli_bev = args_ptr->cli_bev;/////
	struct bufferevent* redis_bev = args_ptr->redis_bev;/////
	struct event_base* my_base = args_ptr->my_base;/////
	int has_to_be = args_ptr->check_cnt;
	int done_flag = args_ptr->done_flag;

	//printf("redis_readcb\n");
	while(done_flag != 1) ;

	//printf("redis callback function start!---------------------\n");
	// cli // receive reply from redis server, 4: read
	if(error_flag == 0){
		//printf("get redis reply\n");
		memset(redis_reply, 0, sizeof(redis_reply));
		memset(final_redis_reply, 0, sizeof(final_redis_reply));
		//- sleep(10);///
		//printf("here\n");
		if(req_flag == 2){ // GET case
			bufferevent_read(redis_bev, final_redis_reply, sizeof(final_redis_reply));
		}
		else{ // SET case
			bufferevent_read(redis_bev, final_redis_reply, sizeof(final_redis_reply));/////
			//printf("here\n");
			//printf("has to be: %d check_cnt: %d\n", has_to_be, check_cnt);
			check_cnt += how_many(final_redis_reply);
			//printf("check_cnt: %d\n", check_cnt);
			while(has_to_be != check_cnt){
				//printf("receive!\n");
				recv(ser_sockfd_1, final_redis_reply, sizeof(final_redis_reply), 0);
			       	check_cnt += how_many(final_redis_reply);	
			}
			//printf("check_cnt: %d\n", check_cnt);
			
			//printf("\n\nfinal redis reply: %s check_cnt: %d\n\n", final_redis_reply, check_cnt);		
		}
		//printf("came out!\n");
	}
	close(ser_sockfd_1);	
	// cli // deliver this reply to client, 5: write
	//----------------------------------------------------------
	//    make reply format to send to client
	//----------------------------------------------------------
	memset(reply_to_cli, 0, MAXBUFLEN);
	if(req_flag == 1){ // SET case
		//printf("-----SET CASE-----\n");
		if(error_flag == 1){
		 	strcpy(reply_to_cli, "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: 5\r\n\r\nERROR");		
		} 
		else if(strstr(final_redis_reply, "OK") != NULL){ // success
			 strcpy(reply_to_cli, "HTTP/1.1 200 OK\r\nContent-Type: test/plain\r\nContent-Length: 2\r\n\r\nOK");
		 }
		 else{ // error
		 	strcpy(reply_to_cli, "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: 5\r\n\r\nERROR");
		 }
	}
	else if(req_flag == 2){ // GET case
		printf("-----GET CASE-----, reply to client\n");
		if(strstr(final_redis_reply, "$-1") != NULL){
		 	strcpy(reply_to_cli, "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: 5\r\n\r\nERROR");
				}
		else if(strstr(final_redis_reply, "$") != NULL){ // success
			// get string from redis_reply
		 	//printf("success!\n");
			found_str = final_redis_reply + 1; //$
			//printf("found_str: %s\n", found_str);
			while((*found_str)!='\n'){//copy value length
				len_buf[cnt] = (*found_str);
				cnt++;
				found_str++;
			}
			value_len = atoi(len_buf);
			//printf("value_len: %d\n", value_len);
			//now, found_str is at '\n'
			found_str++;
			strtok(found_str, "\r");
			strncpy(value_buf, found_str, strlen(found_str)); // copy first one
			//printf("found_str: %s, len: %zu\n", found_str, strlen(found_str));
			//printf("value_buf: %s\n", value_buf);
			//printf("hi\n");
			//printf("value_buf len: %zu\n", strlen(value_buf));
			while(value_len != strlen(value_buf)){
				memset(redis_reply, 0, MAXBUFLEN);
				recv(ser_sockfd_1, redis_reply, MAXBUFLEN, 0);//read more
				strtok(redis_reply, "\r");	
				//printf("redis reply: %s %zu\n", redis_reply, strlen(redis_reply));
				strcat(value_buf, redis_reply);
				//printf("value buf len: %zu\n", strlen(value_buf));

			}
			sprintf(reply_to_cli, "HTTP/1.1 200 OK\r\nContent-Type: test/plain\r\nContent-Length: %d\r\n\r\n%s", value_len, value_buf);
		 }
		 else{ // error
		 	strcpy(reply_to_cli, "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: 5\r\n\r\nERROR");
		 }
	}
	else{ // Not SET or GET case
		 	strcpy(reply_to_cli, "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: 5\r\n\r\nERROR");
				 }
	//----------------------------------------------------------
	//    send reply to client
	//----------------------------------------------------------
	//printf("reply to cli: %s\n", reply_to_cli);///
	//bufferevent_write(cli_bev, reply_to_cli, sizeof(reply_to_cli));
	while(write(cli_sockfd, reply_to_cli, sizeof(reply_to_cli)) == -1) ;
	//ev_accept = event_new(base, my_sockfd, EV_READ|EV_PERSIST, accept_callback, (struct Multi_arg*)multiple_arg);
	//event_add(ev_accept, NULL);	
	//printf("redis event done!!------------------\n");
	//- close(cli_sockfd);
	//---close(ser_sockfd_1);
	//- pthread_mutex_lock(&mutex);
	bufferevent_free(cli_bev);/////
	event_base_loopexit(my_base, NULL);/////
	//- pthread_mutex_unlock(&mutex);
	//printf("here\n");
	//event_free(event_self_cbarg());
	//printf("closed ser_sockfd!!------------------\n");
}

//----------------------------------------------------------
// a thread does its job < th_pool_func < ev_accept
//----------------------------------------------------------
void cli_readcb(struct bufferevent* bev, void* ptr){
//----------------------------------------------------------
	//- int cli_len;
	//- int my_sockfd = mysockfd;
	//- struct sockaddr_in cli_addr = multi_ptr->pass_cli_addr;//
	struct event* ev_cli_1;
	struct event* ev_cli_2;
	struct event* ev_redis;	
	
	struct ev_redis_args* redi_call_args = (struct ev_redis_args*)malloc(sizeof(struct ev_redis_args));
	//---------------create thread-----------------------------
	//multiple_arg->my_pthread = pthread[thread_num];
	//pthread_create(&pthread[thread_num], NULL, t_func, (void*)(multiple_arg));
	//thread_num++;
	// parent process
	//int cli_sockfd = multi_ptr->cli_fd;
	//pthread_t thread_self = multi_ptr->my_pthread;
	//int cli_sockfd = multi_ptr->cli_fd;
	//----------------------------------------------------------
	//----------------------------------------------------------
	char req_from_cli[MAXBUFLEN];
	struct evbuffer* rough_req = (struct evbuffer*)malloc(MAXBUFLEN);
	int req_len_cli;
	char *method, *url, *ver, *body;
	char* str_helper;
	char* sub_helper;
	char* redis_req = (char*)malloc(sizeof(char)*MAXBUFLEN);
	char *set_helper, *get_helper;
	//int word_count;
	char word_buf[MAXBUFLEN];
	int req_flag; // 1: post, 2: get
	//char reply_to_cli[MAXBUFLEN];
	//----------------------------------------------------------
	char key_buf[MAXBUFLEN];
	char value_buf[MAXBUFLEN];
	char *key_helper, *value_helper;
	int end_flag = 0;
	//int full_flag = 0;
	int start_flag = 0;
	int pkt_end_flag = 0;
	int key_flag = 1;
	int still_working = 0;
	int error_flag = 0;
	int key_cnt, value_cnt;
	int found_eq_sign = 0;
	char* content_len_str;
	char content_len_buf[100]; 
	int i = 0;
	int whole_size;
	int read_left;
	//int reply_cnt;
	//----------------------------------------------------------
	evutil_socket_t ser_sockfd;
	evutil_socket_t ser_sockfd2;
	evutil_socket_t ser_sockfd3;
	evutil_socket_t ser_sockfd4;
	struct sockaddr_in ser_addr;
	struct hostent* ser_ip;
	int ser_port;
	//char redis_reply[MAXBUFLEN];
	//char final_redis_reply[MAXBUFLEN];
	//char* found_str;
	////////////////////////////////////////////////////////////
	struct bufferevent* redis_bev;
	struct redis_buf_ARGS* redis_b_arg = (struct redis_buf_ARGS*)malloc(sizeof(struct redis_buf_ARGS));
	int cli_sockfd = ((struct cli_buf_ARGS*)ptr)->cli_sockfd;
	struct bufferevent* cli_bev = ((struct cli_buf_ARGS*)ptr)->cli_bev;
	struct event_base* my_base;
	int check_cnt = 0;
	////////////////////////////////////////////////////////////
	
	//printf("cli_readcb\n");

	//printf("now, execute accept callback!\n");
	//----------------------------------------------------------
	//    work as a client (to send req to redis)
	//----------------------------------------------------------
	//printf("cli: socket, connect\n");
	// accept client
	//- cli_len = sizeof(cli_addr);
	//- cli_sockfd = accept(my_sockfd, (struct sockaddr*)&cli_addr, &cli_len);
	/*if(cli_sockfd < 0){
		fprintf(stderr, "ser / accept: failed\n");
		exit(1);
	}*/
	//fcntl(cli_sockfd, F_SETFL, O_NONBLOCK);
	//close(my_sockfd);
	
	// cli // 1 : socket
	//printf("cli / socket\n");	
	ser_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(ser_sockfd < 0){
		fprintf(stderr, "cli / socket: failed\n");
		exit(1);
	}
	//fcntl(ser_sockfd, F_SETFL, O_NONBLOCK);
	// cli // 2: connect
	//printf("ser_sockfd: %d\n",ser_sockfd);
	ser_ip = gethostbyname(argv2);
	ser_port = atoi(argv3);
	memset(&ser_addr, 0, sizeof(ser_addr));
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_port = htons(ser_port);
	ser_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*)ser_ip->h_addr));
	
	//connect(ser_sockfd, (struct sockaddr*)&ser_addr, sizeof(ser_addr));
					
	if(connect(ser_sockfd, (struct sockaddr*)&ser_addr, sizeof(ser_addr)) < 0){
			fprintf(stderr, "cli / connect: failed\n");
			exit(1);
	}
	
	///// MAKE ser_sockfd BUFFEREVENT //////////////////////////////////////
	my_base = bufferevent_get_base(cli_bev);/////
	evutil_make_socket_nonblocking(ser_sockfd);
	redis_bev = bufferevent_socket_new(my_base, ser_sockfd, BEV_OPT_CLOSE_ON_FREE);/////
	// set b_arg to pass onto cbs
	redis_b_arg->cli_bev = cli_bev;
	redis_b_arg->redis_bev = redis_bev;
	redis_b_arg->cli_sockfd = cli_sockfd;
	redis_b_arg->ser_sockfd = ser_sockfd;
	redis_b_arg->my_base = my_base;
	redis_b_arg->done_flag = 0;
	redis_b_arg->check_cnt = check_cnt;
	bufferevent_setcb(redis_bev, redis_readcb, redis_writecb, redis_eventcb, (void*)redis_b_arg);/////
	bufferevent_enable(redis_bev, EV_READ|EV_WRITE);/////
	//- bufferevent_setwatermark(redis_bev, EV_READ, );/////
	//- bufferevent_setwatermark(redis_bev, EV_WRITE, );/////
	////////////////////////////////////////////////////////////////////////
	
	// exit_flag : whether there's more packet to receive
	// end flag : until req meets null character
	while((end_flag == 0) && (error_flag == 0)){
		//read_loop_count++;///erase this
		
		error_flag = 0;
		//printf("read from client loop\n");
		//----------------------------------------------------------
		//	if it's the very first request packet
		//----------------------------------------------------------	
		if(start_flag == 0){
			//ev_cli_1 = event_new(base, cli_sockfd, EV_READ|EV_PERSIST, cli_read_callback, (void*)(multiple_arg));
			//event_add(ev_cli_1, NULL);
			//------read from cli callback---------------
			//printf("cli_sockfd: %d\n",cli_sockfd);
			//- while((req_len_cli = recv(cli_sockfd, req_from_cli, MAXBUFLEN, 0)) == - 1) ;
			///// READ FROM INPUT BUFFER ////////////////////////////////
			//printf("here!\n");
			
			usleep(10);//sleep(0.1);
			bufferevent_read(cli_bev, req_from_cli, MAXBUFLEN);
			/////////////////////////////////////////////////////////////
			req_len_cli = strlen(req_from_cli);
			req_from_cli[req_len_cli] = '\0';
			pkt_end_flag = 0;
			
			//printf("req_from_cli: %s %d\n", req_from_cli, req_len_cli);
			//----------------------------------------------------------
			//	parse method and body	
			//----------------------------------------------------------
			if(req_from_cli[0] == 'P') req_flag = 1; // POST
			else req_flag = 2; // GET
					
			redis_b_arg->req_flag = req_flag;/////

			if(req_flag == 1){
				content_len_str = strstr(req_from_cli, "ength") + 7;
				//printf("!!!!!\n");
				//printf("content_len_str: %s\n", content_len_str);
				i = 0;
				while(1){
					if((*content_len_str) == '\r') break;
					content_len_buf[i] = (*content_len_str);
					content_len_str++;
					i++;
				}
				content_len_buf[i] = '\0';
				//printf("content_len_buf: %s\n",content_len_buf);
				// get whole content size
				whole_size = atoi(content_len_buf);
				body = strstr(req_from_cli, "\r\n\r\n") + 4;///
				read_left = whole_size - strlen(body);
				//printf("whole size: %d body: %zu\n", whole_size,strlen(body));
			}
		}
		//----------------------------------------------------------
		//	more reading from client
		//----------------------------------------------------------	
		if(pkt_end_flag == 1){
			//ev_cli_2 = event_new(base, my_sockfd, EV_READ|EV_PERSIST, accept_callback, (void*)(multiple_arg));
			//event_add(ev_cli_2, NULL);
			//- bufferevent_read(cli_bev, req_from_cli, sizeof(req_from_cli));/////
			//printf("pkt end flag is 1, receive more...\n");
			while((req_len_cli = recv(cli_sockfd, req_from_cli, MAXBUFLEN, 0)) == -1) ;
			req_from_cli[req_len_cli] = '\0';
			//printf("more req_from_cli: %s\n",req_from_cli); 
			pkt_end_flag = 0;

			read_left = read_left - strlen(req_from_cli);
		}
				
		//printf("\nread_left: %d\n", read_left);
				
		/////////////////////////////////////////////////////////
		// POST case (SET) //////////////////////////////////////
		/////////////////////////////////////////////////////////
		if(req_flag == 1){ // POST
			//printf("set case\n");///
			//printf("body: %s\n", body);
			//----------------------------------------------------------
			//	save key
			//----------------------------------------------------------
			if(key_flag == 1){
				//printf("save key\n");
				if(start_flag == 0){
					// parse body
					body = strstr(req_from_cli, "\r\n\r\n") + 4;///
					//- printf("body: %p\n", body);
					sub_helper = body;
					start_flag = 1; // means it's not very first loop anymore
				}
				if(still_working == 0){
					key_helper = key_buf;
					memset(key_buf, 0, MAXBUFLEN);
				}
				else{//still_working == 1
					sub_helper = req_from_cli;
				}
						
				//word_count = 1;
				if(still_working == 0) key_cnt = 0;
				found_eq_sign = 0;
						
				still_working = 1;
				while(1){
					if((*sub_helper) == '='){
						key_flag = 0;
						still_working = 0;
						sub_helper++;
						key_helper++;
						found_eq_sign = 1;
						break;
					}
					if((*sub_helper) == '\0'){
						//printf("here!!! found EOF\n");
						pkt_end_flag = 1;
						if(key_buf!=key_helper) key_helper++;
						break;
					}
					if((*sub_helper) == '&'){
						error_flag = 1;
						break;
					}
					if(key_cnt > MAXBUFLEN){
						error_flag = 1;
						break;
					}
					//printf("key_buf: %p, key_helper: %p\n",key_buf, key_helper);
					(*key_helper) = (*sub_helper);
					//-printf("%c\n",(*key_helper));
					sub_helper++;
					key_helper++;
					key_cnt++;
				}	
				//printf("\n\nkey buf: %s len: %zu\n\n", key_buf, strlen(key_buf));
				// send key_buf to redis
				if(still_working == 0){
					//printf("make redis request: key \n");
					check_cnt++;
					if(found_eq_sign == 0) error_flag = 1;
					bufferevent_write(redis_bev, "*3\r\n$3\r\nSET\r\n", strlen("*3\r\n$3\r\nSET\r\n"));/////
					//write(ser_sockfd,"*3\r\n$3\r\nSET\r\n", strlen("*3\r\n$3\r\nSET\r\n"));
					memset(word_buf, 0, MAXBUFLEN);
					sprintf(word_buf, "$%zu\r\n", strlen(key_buf));
					
					redis_b_arg->error_flag = error_flag;/////
				
					bufferevent_write(redis_bev, word_buf, strlen(word_buf));/////
					//write(ser_sockfd, word_buf, strlen(word_buf));
					bufferevent_write(redis_bev, key_buf, strlen(key_buf));/////
					//write(ser_sockfd, key_buf, strlen(key_buf));
					bufferevent_write(redis_bev, "\r\n", strlen("\r\n"));/////
					//write(ser_sockfd, "\r\n", 2);
				}
			}
			//----------------------------------------------------------
			//	save value
			//----------------------------------------------------------
			else{ // key_flag == 0
				//printf("save value\n");
				if(still_working == 0){
					value_helper = value_buf;
					memset(value_buf, 0, MAXBUFLEN);
				}
				else{ //means it's new packet
					sub_helper = req_from_cli;
					//printf("sub_helper reset %c %d\n",(*sub_helper), (*value_helper));
				}
					
				if(still_working == 0) value_cnt = 0;
				still_working = 1;
						
				while(1){
					if((*sub_helper) == '&'){
						key_flag = 1;
						still_working = 0;
						sub_helper++;
						value_helper++;
						break;
					}
					if((*sub_helper) == '\0'){ //packet ended
						//printf("here!\n");
						pkt_end_flag = 1;
						if(read_left == 0) still_working = 2; //it's all done
							//value_helper++;
							break;
					}
					if((*sub_helper) == '='){
						error_flag = 1;
						break;
					}
					if(value_cnt > MAXBUFLEN){
						error_flag = 1;
						break;
					}
					(*value_helper) = (*sub_helper);
					sub_helper++;
					value_helper++;
					value_cnt++;
				}
				//printf("read left: %d, still_working: %d\n",read_left, still_working);
				if((still_working == 0) || (still_working == 2)){
					//printf("make redis request: value\n");
					//printf("\n---------value len: %zu check_cnt: %d\n", strlen(value_buf), check_cnt);
							
					memset(word_buf, 0, MAXBUFLEN);
					sprintf(word_buf, "$%zu\r\n", strlen(value_buf));
					
					//redis_b_arg->error_flag = error_flag;/////
					bufferevent_write(redis_bev, word_buf, strlen(word_buf));/////
					//write(ser_sockfd, word_buf, strlen(word_buf));
					bufferevent_write(redis_bev, value_buf, strlen(value_buf));/////
					//write(ser_sockfd, value_buf, strlen(value_buf));
					bufferevent_write(redis_bev, "\r\n", strlen("\r\n"));/////
					//-- sleep(0.1);
					//write(ser_sockfd, "\r\n", 2);
				}
			}	
			// set end flag
			if((read_left == 0) && (pkt_end_flag == 1)) end_flag = 1;
			if((end_flag == 1) && (key_flag == 1)) error_flag = 1;
			//printf("error_flag: %d\n",error_flag);
		}
		///////////////////////////////////////////////////////
		// GET case (GET) /////////////////////////////////////
		///////////////////////////////////////////////////////
		if(req_flag == 2){
			//printf("get case!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
			end_flag = 1;

			method = strtok(req_from_cli, " ");
			url = strtok(NULL, " ");
			url++;
			//printf("key: %s %zu\n",url, strlen(url));
			memset(redis_req, 0, MAXBUFLEN);
			strcpy(redis_req, "*2\r\n$3\r\nGET\r\n");
			memset(word_buf, 0, MAXBUFLEN);
			sprintf(word_buf, "$%zu\r\n", strlen(url));
			strcat(redis_req, word_buf);
			strcat(redis_req, url);
			strcat(redis_req, "\r\n");
					
			redis_b_arg->error_flag = error_flag;/////
			//printf("here!\n");	
			bufferevent_write(redis_bev, redis_req, strlen(redis_req));/////		
			//printf("here!\n");
			//- while(write(ser_sockfd, redis_req, strlen(redis_req)) == -1) ;
			//printf("GET redis_req: %s\n", redis_req);
		}
	} // end of recv while loop
	//printf("receive ended\n");
	redis_b_arg->done_flag = 1;
	redis_b_arg->check_cnt = check_cnt;	
	//-------------------------------------------------------------------
	// 	           bufferevent(redis read buf)
	//-------------------------------------------------------------------
	//- redi_call_args->ser_sockfd = ser_sockfd;
	//- redi_call_args->cli_sockfd = cli_sockfd;
	//- redi_call_args->error_flag = error_flag;
	//- redi_call_args->req_flag = req_flag;
	//redi_call_args->mysock_fd = my_sockfd;
	//redi_call_args->multi_ptr = multi_ptr;
	//redi_call_args->one_base = base;
	//printf("add redis event\n");
	//- pthread_mutex_lock(&mutex);
	//- ev_redis = event_new(base, ser_sockfd, EV_READ, ev_redis_callback, (void*)(redi_call_args));
	//- event_add(ev_redis, NULL);	
	//- pthread_mutex_unlock(&mutex);
	//event_base_dispatch(base);
	//-------------------------------------------------------------------
			
	// 6: close
	//close(ser_sockfd);
	//close(cli_sockfd);
	
	// detach
	//pthread_detach(thread_self);
	
	// free libevent
	//event_free(ev_redis);
	//close(my_sockfd);
	//printf("accept event done!================================================\n");
	//return 0;		
	//close(ser_sockfd2);
	//close(ser_sockfd3);
	//close(ser_sockfd4);
	
	free(rough_req);	
	free(redis_req);
	//free(ev_redis);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//--------------------------------------------------------------
// waits for task(in task_queue)
// if condition is met, random thread in thread pool goes through routine
//--------------------------------------------------------------
void* thread_pool_func(void* args){
//--------------------------------------------------------------	
	evutil_socket_t cli_sockfd;
	int find_task = 0;
	struct th_ARGS* data = (struct th_ARGS*)args;
	int th_id = data->th_id;
	
	struct bufferevent* cli_bev;/////
	struct cli_buf_ARGS* cli_b_arg = (struct cli_buf_ARGS*)malloc(sizeof(struct cli_buf_ARGS));/////
		
	//printf("thread_pool_func\n");
	
	while(1){
		//printf("--------------------------------th_id 1: %d\n",th_id);
		//- pthread_mutex_lock(&mutex);
		//printf("=================th lock===================\n");
		pthread_mutex_lock(&mutex);//
		pthread_cond_wait(&cond, &mutex);
		//printf("================================th_id 2: %d\n",th_id);
		for(find_task=0; find_task<10; find_task++){	
			if(task_queue[find_task] != 0){
				// get task
				cli_sockfd = task_queue[find_task];
				// dequeue
				task_queue[find_task] = 0;
				///// MAKE cli_sockfd BUFFEREVENT //////////////////////////////////////
				evutil_make_socket_nonblocking(cli_sockfd);
				cli_bev = bufferevent_socket_new(base[th_id], cli_sockfd, BEV_OPT_CLOSE_ON_FREE);/////
				cli_b_arg -> cli_sockfd = cli_sockfd;
				cli_b_arg -> cli_bev = cli_bev;
				bufferevent_setcb(cli_bev, cli_readcb, cli_writecb, cli_eventcb, (void*)cli_b_arg);/////
				bufferevent_enable(cli_bev, EV_READ|EV_WRITE);/////
				
				//bufferevent_setwatermark(cli_bev, EV_READ, );/////
				//bufferevent_setwatermark(cli_bev, EV_WRITE, );/////
				/////////////////////////////////////////////////////////
				// do its job
				//- execute_th_routine(cli_sockfd);
				break;
			}
		}
		//printf("th_id: %d dispatch start!\n",th_id);//
		event_base_dispatch(base[th_id]);
		pthread_mutex_unlock(&mutex);//
		//printf("th_id: %d dispatch finished!\n",th_id);//
	}
	free(cli_b_arg);
}

//--------------------------------------------------------------
// accepts client connection.
// distributes tasks in task_queue(when task_queue entry is zero)
//--------------------------------------------------------------
void accept_cb(int sock, short which, void* args){
//--------------------------------------------------------------
	struct accept_ARGS* data = (struct accept_ARGS*)args;
	struct sockaddr_in cli_addr = data->cli_addr;
	int mysockfd = data->mysockfd;
	int cli_len = sizeof(cli_addr);
	evutil_socket_t cli_sockfd;

	int task_cnt = 0;
	int res;//
	int flag;
	
	//printf("accept_cb\n");
	//printf("=================accept lock===================\n");
	pthread_mutex_lock(&mutex);//
	usleep(500);
	cli_sockfd = accept(mysockfd, (struct sockaddr*)&cli_addr, &cli_len);
	accept_cnt++;
	//printf(">>>>>>>>>>>>>>>accept_cnt: %d\n",accept_cnt);//
	while(1){ //find empty task_queue entry and insert cli_sockfd. 
		//printf("finding loop\n");
		if(task_queue[task_cnt] == 0){
		       	//printf("found empty!\n");
			//printf("clisockfd: %d\n", cli_sockfd);
			task_queue[task_cnt] = cli_sockfd;
			// pthread_mutex_lock(&mutex);	
			// send signal to thread(in thread pool)
			mutex_flag = 1;//
			//printf("=================accept sig===================\n");
			res = pthread_cond_signal(&cond);
			//printf("=================accept unlock===================\n");
			pthread_mutex_unlock(&mutex);//
			usleep(100);//
			//printf("res: %d---------task_cnt: %d\n",res,task_cnt);
			// pthread_mutex_unlock(&mutex);
			break;
		}
		task_cnt++;
		if(task_cnt == 10) task_cnt = 0; // go over the queue again
	}
}

//--------------------------------------------------------------
// creates event base, dispatch.
// makes ten task threads.
// makes socket, binds, listens for client.
// When client wants connection, accept_cb is invoked.
//--------------------------------------------------------------
int main(int argc, char *argv[]) {
//--------------------------------------------------------------
	struct sockaddr_in cli_addr;

	struct accept_ARGS* ac_arg = (struct accept_ARGS*)malloc(sizeof(struct accept_ARGS));	
	struct th_ARGS* th_arg0 = (struct th_ARGS*)malloc(sizeof(struct th_ARGS));
	struct th_ARGS* th_arg1 = (struct th_ARGS*)malloc(sizeof(struct th_ARGS));
	struct th_ARGS* th_arg2 = (struct th_ARGS*)malloc(sizeof(struct th_ARGS));
	struct th_ARGS* th_arg3 = (struct th_ARGS*)malloc(sizeof(struct th_ARGS));
	struct th_ARGS* th_arg4 = (struct th_ARGS*)malloc(sizeof(struct th_ARGS));
	struct th_ARGS* th_arg5 = (struct th_ARGS*)malloc(sizeof(struct th_ARGS));
	struct th_ARGS* th_arg6 = (struct th_ARGS*)malloc(sizeof(struct th_ARGS));
	struct th_ARGS* th_arg7 = (struct th_ARGS*)malloc(sizeof(struct th_ARGS));
	struct th_ARGS* th_arg8 = (struct th_ARGS*)malloc(sizeof(struct th_ARGS));
	struct th_ARGS* th_arg9 = (struct th_ARGS*)malloc(sizeof(struct th_ARGS));

	int port = atoi(argv[1]);
	int mysockfd, flag;
	
	pthread_attr_t attr;
	int th_cnt = 0;
	int status;

	struct event* ev_accept;

	//printf("main\n");
	// set global variables
	argv2 = argv[2];
	argv3 = argv[3];
	
	///// MAKE EVENT BASE /////
	accept_base = event_base_new();
	base[0] = event_base_new();
	base[1] = event_base_new();
	base[2] = event_base_new();
	base[3] = event_base_new();
	base[4] = event_base_new();
	base[5] = event_base_new();
	base[6] = event_base_new();
	base[7] = event_base_new();
	base[8] = event_base_new();
	base[9] = event_base_new();
	
	// SOCKET //	
	mysockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(mysockfd < 0){
		fprintf(stderr, "socket: failed\n");
		return 2;
	}	
	// make socket NONBLOCK
	flag = fcntl(mysockfd, F_GETFL, 0);
	fcntl(mysockfd, F_SETFL, flag|O_NONBLOCK);
	
	memset(&cli_addr, 0, sizeof(cli_addr));
	cli_addr.sin_family = AF_INET;
	cli_addr.sin_port = htons(port);
	cli_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	// BIND //
	if(bind(mysockfd, (struct sockaddr*)&cli_addr, sizeof(cli_addr)) < 0){
		fprintf(stderr, "ser / bind: failed\n");
		return 2;
	}
	// LISTEN //
	if(listen(mysockfd, BACKLOG) < 0){
		fprintf(stderr, "ser / listen: failed\n");
		return 2;
	}

	// make ten threads(of thread pool) //
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond, NULL);

	//- pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	th_arg0->th_id = 0;
	pthread_create(&thread_pool[0], NULL, thread_pool_func, (struct th_ARGS*)th_arg0);
	th_arg1->th_id = 1;
	pthread_create(&thread_pool[1], NULL, thread_pool_func, (struct th_ARGS*)th_arg1);
	th_arg2->th_id = 2;
	pthread_create(&thread_pool[2], NULL, thread_pool_func, (struct th_ARGS*)th_arg2);
	th_arg3->th_id = 3;
	pthread_create(&thread_pool[3], NULL, thread_pool_func, (struct th_ARGS*)th_arg3);
	th_arg4->th_id = 4;
	pthread_create(&thread_pool[4], NULL, thread_pool_func, (struct th_ARGS*)th_arg4);
	th_arg5->th_id = 5;
	pthread_create(&thread_pool[5], NULL, thread_pool_func, (struct th_ARGS*)th_arg5);
	th_arg6->th_id = 6;
	pthread_create(&thread_pool[6], NULL, thread_pool_func, (struct th_ARGS*)th_arg6);
	th_arg7->th_id = 7;
	pthread_create(&thread_pool[7], NULL, thread_pool_func, (struct th_ARGS*)th_arg7);
	th_arg8->th_id = 8;
	pthread_create(&thread_pool[8], NULL, thread_pool_func, (struct th_ARGS*)th_arg8);
	th_arg9->th_id = 9;
	pthread_create(&thread_pool[9], NULL, thread_pool_func, (struct th_ARGS*)th_arg9);

	///// MAKE EVENT ///// (accept event)
	ac_arg->mysockfd = mysockfd;
	ac_arg->cli_addr = cli_addr;
	ev_accept = event_new(accept_base, mysockfd, EV_READ|EV_PERSIST, accept_cb, (struct accept_ARGS*)ac_arg);
	event_add(ev_accept, NULL);
	
	event_base_dispatch(accept_base); // event loop
	
	// detach thread pool threads
	for(th_cnt=0; th_cnt<10; th_cnt++){
		pthread_join(thread_pool[th_cnt], (void*)&status);
	}
	// close
	close(mysockfd);
	event_free(ev_accept);
	pthread_mutex_destroy(&mutex);	
	// free base also
}
