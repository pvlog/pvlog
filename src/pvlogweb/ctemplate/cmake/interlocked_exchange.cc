#include <windows.h>

int main(int argv, char *argv[]) { 
	volatile LONG once;
	InterlockedExchange(&once, 1);
}
