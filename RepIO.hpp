#ifndef REPLAY_IO_CHANNEL
#define REPLAY_IO_CHANNEL

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include "emp-tool/io/io_channel.h"
#include "emp-tool/utils/hash.h"
using std::string;

#ifdef UNIX_PLATFORM

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <sys/socket.h>

namespace emp {
/** @addtogroup IO
  @{
 */

const int RECORD_MODE=0;
const int REPLAY_MODE=1;

class RepIO: public IOChannel<RepIO> { public: 

	std::vector<char>recv_rec;
	int recv_cur;

	RepIO(const char * address, int port,bool quiet = false) {
        
        recv_cur=0;

        if(!quiet)
			std::cout << "replaying\n";
	}

	void sync() {

	}

	~RepIO(){

	}

	void set_nodelay() {

	}

	void set_delay() {

	}

	void flush() {

	}


    Hash send_hash;
    Hash recv_hash;
    
	void send_data(const void * data, int len) {
		send_hash.put(data,len);
	}

	void recv_data(void  * data, int len) {
		if(recv_cur+len>(int)recv_rec.size())
			error("RepIO recv_data error!\n");
        recv_hash.put(data,len);
		for(int i=0;i<len;i++){
			((unsigned char*)data)[i]=(unsigned char)recv_rec[recv_cur+i];
		}
		recv_cur+=len;
	}
};
/**@}*/

}

#else  // not UNIX_PLATFORM

#include <boost/asio.hpp>
using boost::asio::ip::tcp;

namespace emp {

/** @addtogroup IO
  @{
 */
class NetIO: public IOChannel<NetIO> { 
public:
	bool is_server;
	string addr;
	int port;
	uint64_t counter = 0;
	char * buffer = nullptr;
	int buffer_ptr = 0;
	int buffer_cap = NETWORK_BUFFER_SIZE;
	bool has_send = false;
	boost::asio::io_service io_service;
	tcp::socket s = tcp::socket(io_service);
	NetIO(const char * address, int port, bool quiet = false) {
		this->port = port;
		is_server = (address == nullptr);
		if (address == nullptr) {
			tcp::acceptor a(io_service, tcp::endpoint(tcp::v4(), port));
			s = tcp::socket(io_service);
			a.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
			a.accept(s);
		} else {
			tcp::resolver resolver(io_service);
			tcp::resolver::query query(tcp::v4(), address, std::to_string(port).c_str());
			tcp::resolver::iterator iterator = resolver.resolve(query);

			s = tcp::socket(io_service);
			s.connect(*iterator);
		}
		s.set_option( boost::asio::socket_base::send_buffer_size( 65536 ) );
		buffer = new char[buffer_cap];
		set_nodelay();
		if(!quiet)
			std::cout << "connected\n";
	}
	void sync() {
		int tmp = 0;
		if(is_server) {
			send_data(&tmp, 1);
			recv_data(&tmp, 1);
		} else {
			recv_data(&tmp, 1);
			send_data(&tmp, 1);
			flush();
		}
	}

	~NetIO() {
		flush();
		delete[] buffer;
	}

	void set_nodelay() {
		s.set_option(boost::asio::ip::tcp::no_delay(true));
	}

	void set_delay() {
		s.set_option(boost::asio::ip::tcp::no_delay(false));
	}

	void flush() {
		boost::asio::write(s, boost::asio::buffer(buffer, buffer_ptr));
		buffer_ptr = 0;
	}

	void send_data(const void * data, int len) {
		counter += len;
		if (len >= buffer_cap) {
			if(has_send) {
				flush();
			}
			has_send = false;
			boost::asio::write(s, boost::asio::buffer(data, len));
			return;
		}
		if (buffer_ptr + len > buffer_cap)
			flush();
		memcpy(buffer + buffer_ptr, data, len);
		buffer_ptr += len;
		has_send = true;
	}

	void recv_data(void  * data, int len) {
		int sent = 0;
		if(has_send) {
			flush();
		}
		has_send = false;
		while(sent < len) {
			int res = s.read_some(boost::asio::buffer(sent + (char *)data, len - sent));
			if (res >= 0)
				sent += res;
			else 
				fprintf(stderr,"error: net_send_data %d\n", res);
		}
	}
};

}

#endif  //UNIX_PLATFORM
#endif  //NETWORK_IO_CHANNEL
