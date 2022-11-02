#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include<stdio.h>
#include<string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include<signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#define bool short
#define true 1
#define false 0

#define confFileName "config"
#define adFileName "ad"
#define codeFileName "code"
#define SERV_PORT 25555 //通信端口
#define BACKLOG 20 //
#define BUF_SIZE 256 //缓冲区大小

// 配置消息 
 typedef struct mes{
 	char* method;
 	char* speed;
 	char* text;
 } Mes;
 


enum Direction {
    TO_LEFT, TO_RIGHT, TO_DOWN, TO_UP
};

void jmdelay(int n) {
    int i, j, k;
    for(i=0; i<n; i++)
        for(j=0; j<100; j++)
            for(k=0; k<100; k++);
}

int string2Int(char *cVal) {
    int i;
    int res = 0;
    for(i = 0; i < strlen(cVal); i++) {
        res = res * 10 + (cVal[i] - '0');
    }
    return res;
}

void splitLine(char *line, char *key, char *val) { //split one line into 2 part according to 1st ':'
    int i, j = 0;
    bool nowKey = true;
    for(i = 0; i < strlen(line); i++) {
        if(line[i] != ':') {
	    if(nowKey) {
	        key[j++] = line[i];
	    }else {
	        val[j++] = line[i];
            }
	}else {
	    key[j] = '\0';
	    nowKey = false;
	    j = 0;
	}   
    }
    val[j] = '\0';
    return;
}

void openFile(char *fileName, int *fd) {
    *fd = open(fileName, O_RDWR);
    if(*fd < 0) { //config file does not exist
        *fd = open(fileName, O_CREAT, 0666); //create one
        if(*fd < 0) { //still does not exist(create err e.t.c.);
	    printf("Fatal error when open file.\n");
	    exit(-1);
	}
    }
    return;
}

void readConfig(int fd, int* cIdx, int* speed, int* dir) {
   char buf[500]; memset(buf, 0, sizeof(buf));
   if(read(fd, buf, sizeof(buf)) < 0) {
       printf("Fatal error when reading configuration file.\n");
       exit(-1);
   }
   int i;
   char line[100]; memset(line, 0, sizeof(line));
   char key[100]; memset(key, 0, sizeof(key));
   char cVal[100]; memset(cVal, 0, sizeof(cVal));
   int j = 0;
   int val;
   for(i = 0; buf[i] != '\0'; i++) {
       if(buf[i] != '\n') {
           line[j++] = buf[i];
       }else {
           line[j] = '\0';
	   j = 0;
	   splitLine(line, key, cVal); //split line into command and val
	   val = string2Int(cVal);
	   if(strcmp(key, "cttIdx") == 0) {
	       *cIdx = val;
	   }else if(strcmp(key, "speed") == 0) {
	       *speed = val;
	   }else if(strcmp(key, "dir") == 0) {
	       *dir = val;
	   }
       }
   }
   close(fd);
   return;
}

void getAD(int tgt, char *ad) { //given a number(index), return corresponding ad content
   int adfd;
   openFile(adFileName, &adfd); //open file
 
   char buf[1000]; memset(buf, 0, sizeof(buf));
   if(read(adfd, buf, sizeof(buf)) < 0) {
       printf("Fatal error when reading ad file.\n");
       exit(-1);
   } //read content of ad file
   
   int i;
   char line[100]; memset(line, 0, sizeof(line));
   char cKey[100]; memset(cKey, 0, sizeof(cKey));
   char val[100]; memset(val, 0, sizeof(val));
   int j = 0;
   int key;
   for(i = 0; buf[i] != '\0'; i++) {
       if(buf[i] != '\n') {
           line[j++] = buf[i];
       }else {
           line[j] = '\0';
	   j = 0;
	   splitLine(line, cKey, val);
	   key = string2Int(cKey);
	   if(key == tgt) {
	       strcpy(ad, val);
               close(adfd);
	       return;
	   }
       }
   }

   close(adfd);
   printf("No Matched AD for index %d!\n", tgt);
   exit(-1);
}

void getCode(char tgt, char *code) { //given a char, return corresponding LED code
    int codefd;
    openFile(codeFileName, &codefd);

    FILE *codeStream = fopen(codeFileName, "rb"); //get file length
    fseek(codeStream, 0, SEEK_END);
    int fileLen = ftell(codeStream);
    fseek(codeStream, 0, SEEK_SET);
    fclose(codeStream);

    //DEBUG
    //printf("%d\n", fileLen);
    
    char buf[1000]; memset(buf, 0, sizeof(buf));
    if(read(codefd, buf, sizeof(buf)) < 0) {
        printf("Fatal error when reading code file.\n"); //error handler
        exit(-1);
    } //read content of code file

   int i;
   char line[100]; memset(line, 0, sizeof(line));
   int j = 0, k;
   for(i = 0; i < fileLen; i++) {
       if(buf[i] != '\n') {
           line[j++] = buf[i];
       }else {
	   j = 0;

	   if(line[0] == tgt) {
               for(k = 0; k < 8; k++) {
                   code[k] = line[k+2];
               }
               close(codefd);
	       return;
	   }
       }
   }

   close(codefd);
   printf("No Matched Code for Char %c!\n", tgt);
   exit(-1);
}

void startDisplay() {
    int conffd;
    openFile(confFileName, &conffd);
    int cIdx, speed, dir;
    readConfig(conffd, &cIdx, &speed, &dir); //get configuration
    printf("Reading Configuration Success.\n");
    
    int realSpeed;
    
    /*
        gears: 
            0: 2000
            1: 1800
            ...
            9: 200
        dir:
	    0: to left
	    1: to right
	    2: to up
	    3: to down
    */
    
    char ad[500]; memset(ad, 0, sizeof(ad)); //ad content in char form
    getAD(cIdx, ad);
   
    //DEBUG
    printf("%s %d\n", ad, strlen(ad));
    
    int maxBound = strlen(ad) * 8; //each char => 8 byte
    char dispData[4300]; memset(dispData, 0, sizeof(dispData)); //ad content in byte form

    char code[8]; //code of a single char
    int i, j;
    int ddp = 0;
    for(i = 0; i < strlen(ad); i++) {
        getCode(ad[i], code); //get code of a single char	
	for(j = 0; j < 8; j++) { //merge char code to string code
	    dispData[ddp++] = code[j];
            //printf("%d ", (int)code[j]);
	}
        //printf("\n");
    } 
    printf("Make String Code Finished!\n");    

    int ledfd = open("/dev/s3c2440_led0",O_RDWR);
    if (ledfd < 0) {
        printf("Fatal Error When Open Led Device.\n");
        exit(-1);
    } //open led
 
    //start displaying
    int dispStart = 0;
    char onceData[8];
    int colCounter = 0;
    int vLineCounter = 0;
    char part1, part2;
    while(1) {
        openFile(confFileName, &conffd);
        readConfig(conffd, &cIdx, &speed, &dir); //get configuration
        if(speed >= 0 && speed <= 9) { //valid speed
            realSpeed = (-speed + 10) * 200; //10 gears, the greater, the faster
        }else {
            continue; //pause
        }

        if(dir == TO_RIGHT || dir == TO_LEFT) { //horizontal move
    	    if(dir == TO_LEFT) {	
                for(j = 0; j < 8; j++) { //display unit as one single letter
     	            for(i = 0; i < 8; i++) {
	                onceData[i] = dispData[(dispStart + i) % maxBound];
                    }
                    write(ledfd, onceData, 8);
	            jmdelay(realSpeed);
                    dispStart = (dispStart + 1 + maxBound) % maxBound;
                }
            }else { //to right
	        for(colCounter = 0; colCounter < 8; colCounter++) {
		    for(i = 0; i < 8; i++) {
		        if(i < colCounter) { //next char
			    onceData[i] = dispData[(dispStart + 16 - colCounter + i) % maxBound];
			}else {
			    onceData[i] = dispData[(dispStart + i - colCounter) % maxBound];
			}
                    }
		    write(ledfd, onceData, 8);
		    jmdelay(realSpeed);
		}
	        dispStart = (dispStart + 8) % maxBound;
	    }
        }else { //vertical move
            memset(onceData, 0, sizeof(onceData));
	    for(vLineCounter = 0; vLineCounter < 8 + 2; vLineCounter++) { // 2 blank row
	        for(i = 0; i < 8; i++) { //ith column
		    if(dir == TO_DOWN) { //to down
		        part1 = dispData[(dispStart + i) % maxBound] << vLineCounter;
			part2 = 0;
			int bitClearer = 128;
			for(j = 0; j < 8; j++) {
			    if(j < vLineCounter) {
			         part2 = part2 | (dispData[(dispStart + 8 + i) % maxBound] & bitClearer);
			    }
			    bitClearer = bitClearer >> 1;
			}
			
			if(vLineCounter < 8) {
			    part2 = part2 >> (8 - vLineCounter);
			    part2 = part2 >> 2; //leave 2 blank row
			}else { //wait until normal
			    part2 = part2 >> (10 - vLineCounter);
			}
		    }else { //to up
		        part1 = dispData[(dispStart + i) % maxBound] >> vLineCounter;
			part2 = 0;
			int bitClearer = 1;
			for(j = 0; j < 8; j++) {
			    if(j < vLineCounter) {
			        part2 = part2 | (dispData[(dispStart + 8 + i) % maxBound] & bitClearer);
			    }
			    bitClearer = bitClearer << 1;
			}
			
			if(vLineCounter < 8) {
			    part2 = part2 << (8 - vLineCounter);
			    part2 = part2 << 2; //leave 2 blank row
			}else { //wait till normal
			    part2 = part2 << (10 - vLineCounter);
			}
		    }
		    onceData[i] = part1 | part2;
		}
		write(ledfd, onceData, 8);
		jmdelay(realSpeed);
	    } 
	    dispStart = (dispStart + 8) % maxBound; 
        }
    }
    return;
}
void sighandler(int signum)
{
    printf("Pthread stop singal.\n");
    pthread_exit(0);
}

// 替换为LED显示 
void* print_message_func(void *arg)
{
	signal(SIGALRM, sighandler);
	
 	startDisplay();


}



int main()
{
    int i,ret;
    char buf[BUF_SIZE];
    pid_t pid;
    int sockfd;
    int clientfd;
    fd_set readfds;
    struct sockaddr_in host_addr; //服务器端地址
    struct sockaddr_in client_addr; //客户端地址
    int length=sizeof(client_addr);
    struct timeval tv;
    Mes mes;
    pthread_t LED_thread;
    int flag = 0;
    // create socket
    sockfd=socket(AF_INET,SOCK_STREAM,0); //创建Socket SOCK_STREAM:流套接字 AF_INET: IPV4地址族
    fcntl(sockfd,F_SETFL,O_NONBLOCK); //改变Socket属性 F_SETFL:设置状态标志; NONBLOCK非阻塞
    if(sockfd==-1)
    {
    printf("Socket创建失败!!!");
    return 1;
    }
    for(i=0;i<8;i++)
    {
        host_addr.sin_zero[i]=0x00;
    }
    host_addr.sin_family=AF_INET; //IPV4
    host_addr.sin_port=htons(SERV_PORT); //监听端口
    host_addr.sin_addr.s_addr=INADDR_ANY; //
    ret=bind(sockfd,(struct sockaddr *)&host_addr,sizeof(host_addr)); //绑定
    if(ret==-1)
    {
        printf("bind fail!!!");
        return 1;
    }
    //listen
    ret=listen(sockfd,BACKLOG); //监听
    if(ret==-1)
    {
        printf("listen fail!!!");
        return 1;
    }
    while(1)
    {
        clientfd=accept(sockfd,(struct sockaddr*)&client_addr,&length); //监听客户端连接
        if(clientfd!=-1)
        {
            printf("accept %d\n",clientfd);
            FD_ZERO(&readfds);
            FD_SET(clientfd, &readfds);
            ret = select(FD_SETSIZE, &readfds, NULL, NULL, &tv);
            if(ret>0)
            {
                //pid=fork();
				//printf("pid:%d\n",pid);
                if(1)
{
		    char before_cont[BUF_SIZE]="";
                    while(1){
						printf("in while\n"); 
						bzero(buf,sizeof(buf));
			        	int k = recv(clientfd,buf,sizeof(buf),0); //receive data
			       		printf("recv=%s\n",buf);
			        	printf("k : %d\n",k);
					if(k < 4)break;
					char seps[]="$";
			        	char* token;
			        	char types[]="";
			        	char method[BUF_SIZE] = "dir:";
			        	char speed[BUF_SIZE] = "speed:";
			        	char text[BUF_SIZE] = "cttIdx:";
			        	char adver[BUF_SIZE] ="1:";
					char user_name[BUF_SIZE]="";
			        	char user_pwd[BUF_SIZE]="";
			        	token = strtok(buf,seps);
						if(token != NULL)
			                strcat(method,token);
			            token = strtok(NULL,seps);
			            if(token != NULL)
							strcat(speed,token);
			            token = strtok(NULL,seps);
				char cont[BUF_SIZE]="";		
				if(token != NULL){
				   strcpy(cont,token);
				    strcat(text,cont);
					}
				    token = strtok(NULL,seps);
				   strcat(adver,token);	

					strcat(method,"\n");
                    			strcat(speed,"\n");
                    			strcat(text,"\n");
					strcat(adver,"\n");	
					FILE* fp;
                    		fp = fopen("config","w+");
                    		if(fp==NULL)
                    		{
                       		printf("open file fail!\n");
                    	
				}
				else{
                    		fputs(method,fp);
                    		fputs(speed,fp);
                    		fputs(text,fp);
                    		fclose(fp);
				printf("finish write\n");
				}	
				
				fp = fopen("ad","w+");
                                if(fp==NULL)
                                {
                                printf("open file fail  2!\n");

                                }
                                else{
                                fputs(adver,fp);
                                //fputs(speed,fp);
                                
                                fclose(fp);
                                printf("finish write  2\n");
                                }
				

		//thread
						mes.method = method;
						mes.speed = speed;
						mes.text = text;
				int change = 0;
				if(strlen(before_cont) == 0){
				change = 1;
                                strcpy(before_cont,cont);
				}
				else{
				int l = 0;
				change = strcmp(cont,before_cont);
				strcpy(before_cont,cont);
				}
				if(change != 0){	
						if(flag){
							pthread_kill(LED_thread, SIGALRM); //stop LED thread 
						}
						
						int ret_thread1 = pthread_create(&LED_thread,NULL,print_message_func,NULL);
					    if(ret_thread1 == 0)
					        printf("create thread true\n");
					    else
					        printf("create thread false\n");
					    pthread_detach(LED_thread);
					    flag = 1;
					}
    				}
    				close(clientfd);
    			}//2
    			else if(pid>0) { close(clientfd); };
    		}
    	};
    sleep(1);
    };//1
    return 0;
}


