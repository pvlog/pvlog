#define _XOPEN_SOURCE 500
#include <pthread.h>

int main(int argc, char *argv[]) { 
	pthread_rwlock_t l;
	pthread_rwlock_init(&l, (const pthread_rwlockattr_t*)0);
	pthread_rwlock_rdlock(&l);
}
