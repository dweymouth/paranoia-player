#include <algorithm>
#include <condition_variable>

#ifndef _BLOCKING_QUEUE_H_
#define _BLOCKING_QUEUE_H_

template <typename T>
class CircularBlockingQueue {
	private:
		int capacity;
		int size;
		int write_idx;
		int read_idx;

		std::mutex mut;
		std::condition_variable has_cap_cv;
		T *buf;
		bool read_paused;
	public:
		// Constructs a CircularBlockingQueue that can hold capacity items.
		CircularBlockingQueue(int capacity);

		// Writes data to the queue. Blocks until all data is written.
		int blocking_write(T *data, int len);

		// Reads up to count items from the queue and copies them into data.
		// Returns the number of items read.
		int non_blocking_read(int count, T *data);

		// Returns the number of items currently in the queue.
		int get_count();

		// Returns a value in [0.0, 1.0] representing how full the buffer is.
		float get_fill_ratio();

		// Sets whether or not reads from this queue are paused.
		// When paused, calls to non_blocking_read will always return 0
		// and copy no data. When buffer is full, calls to blocking_write will block.
		void set_read_paused(bool paused);

		// Clears (drops) all data from the queue.
		void clear();
};

template <typename T>
CircularBlockingQueue<T>::CircularBlockingQueue(int capacity)
{
	this->capacity = capacity;
	this->size = this->read_idx = this->write_idx = 0;
	this->buf = new T[capacity];
}

template <typename T>
void CircularBlockingQueue<T>::clear()
{
	std::lock_guard<std::mutex> lk(this->mut);
	this->size = 0;
	this->read_idx = 0;
	this->write_idx = 0;
}

template <typename T>
int CircularBlockingQueue<T>::get_count()
{
	return this->size;
}

template <typename T>
float CircularBlockingQueue<T>::get_fill_ratio()
{
	return ((float)this->size) / this->capacity;
}

template <typename T>
int CircularBlockingQueue<T>::non_blocking_read(int count, T *data)
{
	if (count <= 0) {
		return 0;
	}

	int read_count;
	// atomic section
	{
		std::lock_guard<std::mutex> lk(this->mut);
		if (this->read_paused) {
			return 0;
		}

		int to_read = std::min(count, this->size);
		read_count = to_read;
		// copy data from read_idx up to end of buf
		int first_read_count = std::min(to_read, this->capacity - this->read_idx);
		memcpy(data, this->buf + this->read_idx, first_read_count * sizeof(T));
		this->read_idx = (this->read_idx + first_read_count) % this->capacity;
		to_read -= first_read_count;
		// copy rest of data from beginning of buffer if needed
		if (to_read > 0) {
			memcpy(data + first_read_count, this->buf + this->read_idx, to_read * sizeof(T));
			this->read_idx = (this->read_idx + to_read) % this->capacity;
		}
		this->size -= read_count;
	}
	this->has_cap_cv.notify_one();
	return read_count;
}

template <typename T>
int CircularBlockingQueue<T>::blocking_write(T *data, int len)
{
	int written = 0;
	T *data_ptr = data;
	while (len > 0) {
		// atomic section
		{
			std::unique_lock<std::mutex> lk(this->mut);
			while (this->size >= this->capacity) {
				this->has_cap_cv.wait(lk);
			}
			int can_write = this->capacity - this->size;
			// how much data to write in this iteration
			int to_write = std::min(can_write, len);
			// copy data from write_idx up to end of buf
			int first_write_count = std::min(to_write, this->capacity - this->write_idx);
			memcpy(this->buf + this->write_idx, data_ptr, first_write_count * sizeof(T));
			this->write_idx = (this->write_idx + first_write_count) % this->capacity;
			this->size += first_write_count;
			to_write -= first_write_count;
			len -= first_write_count;
			written += first_write_count;
			data_ptr += first_write_count;
			// copy rest of data to beginning of buffer if needed
			if (to_write > 0) {
				memcpy(this->buf + this->write_idx, data_ptr, to_write * sizeof(T));
				this->write_idx = (this->write_idx + to_write) % this->capacity;
				this->size += to_write;
				len -= to_write;
				written += to_write;
				data_ptr += to_write;
			}
		}
	}
	return written;
}

template <typename T>
void CircularBlockingQueue<T>::set_read_paused(bool paused)
{
	std::lock_guard<std::mutex> lk(this->mut);
	this->read_paused = paused;
}

#endif
