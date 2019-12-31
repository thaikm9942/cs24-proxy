CC = clang-with-asan
CFLAGS = -Wall -Wextra -Iinclude
LDFLAGS = -lpthread

all: bin/proxy bin/test-cache bin/test-queue

run: bin/proxy
	$(shell killall -9 proxy 2> /dev/null || true)
	./bin/proxy $(shell cs24-port)

test-http:
	bash -c "./tests/sequential.sh tests/http-sites"

test-repeat:
	bash -c "./tests/sequential.sh tests/repeat-sites"

test-concurrent:
	bash -c "timeout -s 9 8s ./tests/concurrent.sh"

out/%.o: src/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

bin/proxy: out/proxy.o out/buffer.o out/client_thread.o out/queue.o out/hash.o
	$(CC) $(CFLAGS) $^ -o $@

bin/test-cache: out/buffer.o out/queue.o out/hash.o out/test-cache.o
	$(CC) $(CFLAGS) $^ -o $@

bin/test-queue: out/buffer.o out/queue.o out/test-queue.o
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f out/*.o bin/proxy
