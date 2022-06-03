#include "blocking_queue.h"

#include <iostream>
#include <thread>
#include <unistd.h>

CircularBlockingQueue<int> queue(5);

void reader()
{
	for (int i = 0; i < 20; i++) {
		int buf[2];
		int read = queue.non_blocking_read(2, buf);
		for (int j = 0; j < read; j++) {
			std::cout << buf[j] << std::endl;
		}
		sleep(1);
	}
}

void writer()
{
	int buf[2];
	for (int i = 0; i < 40; i += 2) {
		buf[0] = i;
		buf[1] = i + 1;
		queue.blocking_write(buf, 2);
	}
}

int main()
{
	std::thread read_t(reader);
	std::thread write_t(writer);
	read_t.join();
	write_t.join();
	return 0;
}
