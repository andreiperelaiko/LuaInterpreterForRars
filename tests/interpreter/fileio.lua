fd = open("tests/interpreter/string.lua")
print(fd)

s = read(fd, 128)
print(s)

lseek(fd, 0, 0)
sagain = read(fd, 7)
print(sagain)

close(fd)

write(1, "write syscall works\n")
