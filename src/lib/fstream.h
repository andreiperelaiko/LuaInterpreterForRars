int open(const char* path);
void close(int);
int write(int fd, char* buffer, int max_length);
int read(int fd, char* buffer, int max_length);
int lseek(int fd, int offset, int whence);