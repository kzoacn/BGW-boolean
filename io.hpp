#ifndef IO_CHANNEL_H__
#define IO_CHANNEL_H__

#ifdef WIN32
 #error "can not use it on windows"
#endif

#include <string> 
const int NETWORK_BUFFER_SIZE=65536;


template<typename T>
class IOChannel {
public:
	void send_data(const void * data, int nbyte) {
		derived().send_data(data, nbyte);
	}
	void recv_data(void * data, int nbyte) {
		derived().recv_data(data, nbyte);
	}
	
	void send_string(const std::string &j) {
		int size=j.length();
		derived().send_data(&size, 4);
		derived().send_data(j.c_str(), size);
	}
	void recv_string(std::string &j) {
		int size;
		derived().recv_data(&size, 4);
		char *str=new char[size+1];
		str[size]=0;
		derived().recv_data(str, size);
		j=std::string(str);
		delete[] str;
	}
  

private:
	T& derived() {
		return *static_cast<T*>(this);
	}
};


#endif// IO_CHANNEL_H__


#ifndef NETWORK_IO_CHANNEL
#define NETWORK_IO_CHANNEL

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string> 
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
using std::string;


class NetIO: public IOChannel<NetIO> { public:
	bool is_server;
	int mysocket;
	int consocket;
	FILE * stream;
	char * buffer;
	bool has_sent;
	string addr,client_ip;
	int port;
	uint64_t counter;
	struct sockaddr_in dest;
	struct sockaddr_in serv;
		
	NetIO(const char * address, int port, int tmout=2000,bool quiet = false) {


		mysocket = -1;	
		consocket = -1;
		stream = NULL;
		buffer = NULL;
		has_sent = false;
		counter = 0;

		timeval timeout;
		timeout.tv_sec=0;
		timeout.tv_usec=tmout*1000;

		this->port = port;
		is_server = (address == NULL);
		socklen_t socksize = sizeof(struct sockaddr_in);
		if (address == NULL) {
			memset(&serv, 0, sizeof(serv));
			serv.sin_family = AF_INET;
			serv.sin_addr.s_addr = htonl(INADDR_ANY); /* set our address to any interface */
			serv.sin_port = htons(port);           /* set the server port number */    
			mysocket = socket(AF_INET, SOCK_STREAM, 0);
			int reuse = 1;
			setsockopt(mysocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse));

			if(tmout){
				setsockopt(mysocket,SOL_SOCKET,SO_SNDTIMEO,&timeout,sizeof(timeout));
				setsockopt(mysocket,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));
			}

			if(bind(mysocket, (struct sockaddr *)&serv, sizeof(struct sockaddr)) < 0) {		
				fprintf(stderr,"[!ERROR!] bind error!!!");
				throw -1;
			}
			if(listen(mysocket, 30) < 0) {		
				fprintf(stderr,"[!ERROR!] listen error!!!");
				throw -1;
			}
		}
		else {
			addr = string(address);
			 
			memset(&dest, 0, sizeof(dest));
			dest.sin_family = AF_INET;
			dest.sin_addr.s_addr = inet_addr(address);
			dest.sin_port = htons(port);


			int cnt=tmout/10;
			while(1) {
				consocket = socket(AF_INET, SOCK_STREAM, 0);

				if (connect(consocket, (struct sockaddr *)&dest, sizeof(struct sockaddr)) == 0) {
			
					break;
				}
				
				close(consocket);
				usleep(10000);
				cnt--;
				if(cnt<=0){
					fprintf(stderr,"[!ERROR!] connect timeout!!!");
					throw -1;
				}
			}
			if(!quiet)
				std::cout << "connected\n";
		}

		if(tmout){
			setsockopt(consocket,SOL_SOCKET,SO_SNDTIMEO,&timeout,sizeof(timeout));
			setsockopt(consocket,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));
		}
		
	}

	void accepting(bool quiet = true){
		if(consocket>=0){
			close(consocket);
		}
		socklen_t socksize = sizeof(struct sockaddr_in);
		
		if(!quiet)
			std::cerr<<"accepting"<<std::endl;
		while(1){
			consocket = accept(mysocket, (struct sockaddr *)&dest, &socksize);
			if(consocket<0){
				if(!quiet)
					fprintf(stderr,"accept error");
				throw -1;
			}else{
				break;
			}
		}
			
		
		client_ip=inet_ntoa(dest.sin_addr);
		if(!quiet)	
			std::cout<<inet_ntoa(dest.sin_addr)<<" connect to server"<<std::endl;
	}


	~NetIO(){
		fflush(stream);
		if(mysocket>=0)
			close(mysocket);
		if(consocket>=0){
			close(consocket);
		}

	}
 
	void send_data(const void * data, int len) {
		write(consocket,data,len);
		counter += len;
	}

	void recv_data(void  * data, int len) {
		if(recv(consocket,data,len,MSG_WAITALL)!=len){
			fprintf(stderr,"[!ERROR!] recv error!!!");
			throw -1;
		}
		
	}
};


#endif  //NETWORK_IO_CHANNEL