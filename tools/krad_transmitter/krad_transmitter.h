#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <arpa/inet.h>

#include "krad_radio_version.h"
#include "krad_system.h"
#include "krad_ring.h"


#ifndef KRAD_TRANSMITTER_H
#define KRAD_TRANSMITTER_H

#define DEFAULT_MAX_RECEIVERS_PER_TRANSMISSION 256
#define DEFAULT_MAX_TRANSMISSIONS 48
#define TOTAL_RECEIVERS DEFAULT_MAX_RECEIVERS_PER_TRANSMISSION * DEFAULT_MAX_TRANSMISSIONS
#define DEFAULT_TRANSMITTER_PORT "8080"
#define KRAD_TRANSMITTER_SERVER APPVERSION

#define KRAD_TRANSMITTER_MAXEVENTS 64
#define DEFAULT_RING_SIZE 160000000
#define DEFAULT_BURST_SIZE 64000

typedef enum {
	IS_FILE = 3150,
	IS_TCP,
} krad_transmission_receiver_type_t;


typedef struct krad_transmitter_St krad_transmitter_t;
typedef struct krad_transmission_St krad_transmission_t;
typedef struct krad_transmission_receiver_St krad_transmission_receiver_t;

struct krad_transmitter_St {

	char not_found[256];
	int not_found_len;

	int port;
	struct sockaddr_in local_address;

	int incoming_connections_sd;
	int incoming_connections_efd;

	krad_transmission_t *krad_transmissions;
	
	pthread_t listening_thread;
	pthread_rwlock_t krad_transmissions_rwlock;
	
	struct epoll_event *incoming_connection_events;
	struct epoll_event event;
	
	int listening;
	int stop_listening;
	
	krad_transmission_receiver_t *krad_transmission_receivers;
	
};

struct krad_transmission_St {

	krad_transmitter_t *krad_transmitter;
	int active;
	int ready;
	char sysname[256];
	char content_type[256];	

	char http_header[256];
	uint64_t http_header_len;

	unsigned char *header;
	uint64_t header_len;

	int connections_efd;
	
	struct epoll_event *transmission_events;
	struct epoll_event event;

	krad_ringbuffer_t *ringbuffer;
	
	unsigned char *test_buffer;
	

	uint64_t position;	
	uint64_t sync_point;
	

	krad_transmission_receiver_t *ready_receivers;
	krad_transmission_receiver_t *ready_receivers_head;
	krad_transmission_receiver_t *ready_receivers_tail;
	int ready_receiver_count;


	pthread_t transmission_thread;
};

struct krad_transmission_receiver_St {

	krad_transmitter_t *krad_transmitter;
	krad_transmission_t *krad_transmission;

	krad_transmission_receiver_type_t krad_transmission_receiver_type;

	struct epoll_event event;

	int fd;

	char buffer[256];
	uint64_t bufpos;

	int wrote_http_header;
	int wrote_header;


	int ready;
	int active;
	int destroy;

	krad_transmission_receiver_t *prev;
	krad_transmission_receiver_t *next;

};


//	program can add file/socket fd's to output streams w/o sillyness thanks to epoll thread safe ok
//	program writes to output stream ringbuffer, and also sets headers and sync points
//	thread for listening for incoming connections and determining what they want
//	thread per output stream

void set_socket_nonblocking (int sd);

krad_transmission_receiver_t *krad_transmitter_receiver_create (krad_transmitter_t *krad_transmitter, int fd);
void krad_transmitter_receiver_destroy (krad_transmission_receiver_t *krad_transmission_receiver);

int krad_transmitter_transmission_transmit (krad_transmission_t *krad_transmission, krad_transmission_receiver_t *krad_transmission_receiver);

void *krad_transmitter_transmission_thread (void *arg);
void *krad_transmitter_listening_thread (void *arg);

void krad_transmitter_receiver_attach (krad_transmission_receiver_t *krad_transmission_receiver, char *request);
void krad_transmitter_handle_incoming_connection (krad_transmitter_t *krad_transmitter, krad_transmission_receiver_t *krad_transmission_receiver);


void krad_transmission_add_ready (krad_transmission_t *krad_transmission, krad_transmission_receiver_t *krad_transmission_receiver);
void krad_transmission_remove_ready (krad_transmission_t *krad_transmission, krad_transmission_receiver_t *krad_transmission_receiver);

krad_transmission_t *krad_transmitter_transmission_create (krad_transmitter_t *krad_transmitter, char *name, char *content_type);
void krad_transmitter_transmission_destroy (krad_transmission_t *krad_transmission);

void krad_transmitter_transmission_set_header (krad_transmission_t *krad_transmission, unsigned char *buffer, int length);
void krad_transmitter_transmission_add_header (krad_transmission_t *krad_transmission, unsigned char *buffer, int length);
void krad_transmitter_transmission_sync_point (krad_transmission_t *krad_transmission);
int krad_transmitter_transmission_add_data (krad_transmission_t *krad_transmission, unsigned char *buffer, int length);

// convient for handlign the open/close of file?
// krad_transmitter_transmission_add_file (krad_transmission_t *krad_transmission, char *filename)
// krad_transmitter_transmission_remove_file (krad_transmission_t *krad_transmission, char *filename)

//void krad_transmitter_transmission_add_fd (krad_transmission_t *krad_transmission, int fd);
// kicking ppl? 
//void krad_transmitter_transmission_remove_fd (krad_transmission_t *krad_transmission, int fd);



int krad_transmitter_listen_on (krad_transmitter_t *krad_transmitter, int port);
void krad_transmitter_stop_listening (krad_transmitter_t *krad_transmitter);

krad_transmitter_t *krad_transmitter_create ();
void krad_transmitter_destroy (krad_transmitter_t *krad_transmitter);

#endif
