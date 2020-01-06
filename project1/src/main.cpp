#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <thread>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
// #include <stdbool.h>
// #include <assert.h>
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
// #include <condition_variable>
// #include <mutex>


using namespace std;

#define TUPLE_SIZE      (100)
#define KEY_SIZE        (10)
#define THREAD_NUM      (32)
#define FLAG(x)         ((1 << x) - 1)
#define LIMIT_MEM       (1000000000ULL)
#define BLOCK_SIZE      (LIMIT_MEM / THREAD_NUM)
#define BUFFER_SIZE     (10000000)
#define FILE_DIV		(10)
#define REMAIN			(8)

int compare(const void* p1, const void* p2){
	int idx = 0;
	while(*(unsigned char*)(p1 + idx) - *(unsigned char*)(p2 + idx) == 0 && ++idx < KEY_SIZE) ;
	return *(unsigned char*)(p1 + idx) - *(unsigned char*)(p2 + idx);
}


class myHeap{
public:
	int size;
	int arr[THREAD_NUM  + 1];
	char * buf;
	size_t * partial_offset;

	myHeap(char * buf, size_t * partial_offset) : size(0), buf(buf), partial_offset(partial_offset) {
		memset(arr, 0, sizeof(arr));
	}
	bool empty(){
		return size == 0;
	}
	void push(int idx){
		arr[++size] = idx;
		heapify(0);
	}
	int pop(){
		int ret = arr[1];
		arr[1] = arr[size--];
		heapify(1);
		return ret;
	}
private:
	void swap(int &a, int &b){
		int temp = a;
		a = b;
		b = temp;
	}
	void heapify(int is_pop){
		if(is_pop){
			int idx = 1;
			int next = idx * 2;
			while(next <= size){
				if(next == size){
					if(compare(buf + partial_offset[arr[idx]], buf + partial_offset[arr[next]]) > 0) swap(arr[idx], arr[next]);
					break;
				}
				if(compare(buf + partial_offset[arr[next]], buf + partial_offset[arr[next + 1]]) > 0){
					next = next + 1;
				}
				if(compare(buf + partial_offset[arr[idx]], buf + partial_offset[arr[next]]) > 0) swap(arr[idx], arr[next]);
				else break;
				idx = next;
				next = idx * 2;
			}
		}
		else{
			int idx = size;
			int next;
			while(idx > 1){
				next = idx / 2;
				if(compare(buf + partial_offset[arr[idx]], buf + partial_offset[arr[next]]) < 0) swap(arr[idx], arr[next]);
				else break;
				idx = next;
			}
		}

	}
};

typedef struct tup{
	unsigned char data[100];
} Tup;

vector<const char*> temp_file_name;

int file_read(int fd, char *buf, size_t size, size_t start);
int file_write(int fd, char *buf, size_t size, size_t start);

const char * make_temp_file(int block_no, int index){ 
	string *s = new string("B");
	*s += to_string(block_no);
	*s += "T";
	*s += to_string(index);
	return s->c_str();
}


bool compare_sort(const Tup &a, const Tup& b){
	int idx = 0;
	while(a.data[idx] - b.data[idx] == 0 && ++idx < KEY_SIZE) ;
	return a.data[idx] - b.data[idx] < 0;
}

void worker(const char *path, char * buf, size_t size, int index){
	int input_fd = open(path, O_RDONLY);
	file_read(input_fd, buf, size, index * size);
	close(input_fd);
	sort((Tup*)buf, (Tup*)buf + size / TUPLE_SIZE, compare_sort);
	
}

int main(int argc, char* argv[]) {

	if (argc < 3) {
		printf("usage: ./run InputFile OutputFile\n");
		return 0;
	}

	// open input file.
	int input_fd;
	input_fd = open(argv[1], O_RDONLY);
	if (input_fd == -1) {
		printf("error: open input file\n");
		return 0;
	}

	// get size of input file.
	size_t file_size;
	file_size = lseek(input_fd, 0, SEEK_END);
	close(input_fd);
	size_t num_of_divide = file_size / BLOCK_SIZE;
	int file_num = num_of_divide / THREAD_NUM;
	if(file_num > 1){
		for(int i = 0; i < file_num; ++i){
			temp_file_name.push_back(make_temp_file(1, i));
		}
	}
	

	char * data = new char[LIMIT_MEM];
	char * buffer = new char[BUFFER_SIZE];
	size_t partial_offset[THREAD_NUM];
	size_t buffer_size = REMAIN * BUFFER_SIZE * FILE_DIV / file_num;
	char **temp_buf = new char*[file_num];
	if(file_num > 1){
		temp_buf = new char*[file_num];
		for(int i = 0; i < file_num; ++i){
			temp_buf[i] = new char[buffer_size];
		}
	}
	
	
	
	for(int i = 0; i < file_num /* <- temp value*/; ++i){

		vector<thread> workers;
		int k = THREAD_NUM * i;

		for(int j = 0; j < THREAD_NUM; ++j){
			partial_offset[j] = j * BLOCK_SIZE;
			workers.emplace_back(worker, argv[1], data + j * BLOCK_SIZE, BLOCK_SIZE, (k + j));

		}
		for(int j = 0; j < THREAD_NUM; ++j){
			workers[j].join();
		}
		//break;
		int out_fd;
		if(file_num <= 1){
			out_fd = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, 0777);
			if(out_fd == -1){
				printf("error: open temp%d file\n", i);
				return 0;
			}
		}
		else{
			out_fd = open(temp_file_name[i], O_RDWR | O_CREAT | O_TRUNC, 0777);
			if(out_fd == -1){
				printf("error: open temp%d file\n", i);
				return 0;
			}
		}

		int flag = 0;
		int idx = 0;
		size_t buffer_offset = 0;
		size_t output_offset = 0;
		myHeap h(data, partial_offset);
		for(int t = 0; t < THREAD_NUM; ++t){
			h.push(t);
		}
		bool fflag = file_num <= 1 ? true : false;
		while((flag & FLAG(THREAD_NUM)) != FLAG(THREAD_NUM)){
			idx = h.pop();
			if(!fflag) memcpy(temp_buf[i] + buffer_offset, data + partial_offset[idx], TUPLE_SIZE);
			else memcpy(buffer + buffer_offset, data + partial_offset[idx], TUPLE_SIZE);
			buffer_offset = buffer_offset + TUPLE_SIZE;
			partial_offset[idx] = partial_offset[idx] + TUPLE_SIZE;
			if(!fflag && buffer_offset >= buffer_size){
				fflag = true;
				output_offset = output_offset + buffer_size;
				buffer_offset = 0;
			}
			else if(fflag && buffer_offset >= BUFFER_SIZE){
				if(file_write(out_fd, buffer, BUFFER_SIZE, output_offset) < 0) printf("error: write temp%d file\n", i);
				output_offset = output_offset + BUFFER_SIZE;
				buffer_offset = 0;
			}
			if(partial_offset[idx] >= BLOCK_SIZE * (idx + 1)){
				flag = flag | (1 << idx);
				continue;
			}
			h.push(idx);
		}
		close(out_fd);
	}
	delete[] data;
	delete[] buffer;
	if(file_num <= 1) return 0;
	int *temp_fd = new int[file_num];
	for(int i = 0; i < file_num; ++i){
		temp_fd[i] = open(temp_file_name[i], O_RDONLY);
		if(temp_fd[i] == -1){
			printf("error: open temp%d file\n", i);
			return 0;
		}
	}

	
	size_t *temp_in_offset = new size_t[file_num];
	memset(temp_in_offset, 0, sizeof(temp_in_offset) * file_num);
	size_t *temp_used = new size_t[file_num];
	memset(temp_used, 0, sizeof(temp_used) * file_num);
	

	size_t temp_out_offset = 0;
	int flag = 0;
	size_t offset = 0;
	int output_fd = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, 0777);
	if(output_fd == -1){
		printf("error: open output file\n");
		return 0;
	}
	char *write_buf = new char[BUFFER_SIZE];
	int idx = 0;
	

	while((flag & FLAG(file_num)) != FLAG(file_num)){
		for(int i = 0; i < file_num; i++){
			if(i == idx || flag & (1 << i)) continue;
			if(compare(temp_buf[idx] + temp_in_offset[idx], temp_buf[i] + temp_in_offset[i]) > 0) idx = i;
		}
		memcpy(write_buf + offset, temp_buf[idx] + temp_in_offset[idx], TUPLE_SIZE);
		offset = offset + TUPLE_SIZE;
		temp_in_offset[idx] = temp_in_offset[idx] + TUPLE_SIZE;
		if(offset >= BUFFER_SIZE){

			if(file_write(output_fd, write_buf, BUFFER_SIZE, temp_out_offset) < 0) printf("error: write output file\n");

			temp_out_offset = temp_out_offset + BUFFER_SIZE;
			offset = 0;
		}
		if(temp_in_offset[idx] + temp_used[idx] >= LIMIT_MEM){
			temp_used[idx] = temp_used[idx] + temp_in_offset[idx];
			temp_in_offset[idx] = 0;
			flag = flag | (1 << idx);
			for(int i = 0; i < file_num; ++i){
				if(!(flag & (1 << i))){
					idx = i;
					break;
				}
			}
		}
		else if(temp_in_offset[idx] >= buffer_size){
			temp_used[idx] = temp_used[idx] + temp_in_offset[idx];
			temp_in_offset[idx] = 0;
			size_t rsz = buffer_size > (LIMIT_MEM - temp_used[idx]) ? (LIMIT_MEM - temp_used[idx]) : buffer_size;
			if(file_read(temp_fd[idx], temp_buf[idx], rsz, temp_used[idx]) < 0) printf("error: read temp%d file\n", idx);
		}
	}
	for(int i = 0; i < file_num; i++) delete[] temp_buf[i];
	delete[] temp_buf;
	delete[] write_buf;
	delete[] temp_used;
	delete[] temp_in_offset;


	for(int i = 0; i < file_num; i++) close(temp_fd[i]);
	delete[] temp_fd;
	close(output_fd);

	return 0;
}

int file_read(int fd, char *buf, size_t size, size_t start){
	for(size_t offset = 0; offset < size; ){
		size_t ret = pread(fd, buf + offset, size - offset, start + offset);
		if(ret < 0) return -1;
		offset = offset + ret;
	}
	return 1;
}

int file_write(int fd, char *buf, size_t size, size_t start){
	for (size_t offset = 0; offset < size; ) {
		size_t ret = pwrite(fd, buf + offset, size - offset, start + offset);
		if (ret < 0) return -1;
		offset = offset + ret;
	}
	return 0;
}