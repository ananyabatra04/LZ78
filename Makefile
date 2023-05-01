CC = clang
CFLAGS = -Wall -Wpedantic -Werror -Wextra -Wstrict-prototypes
COMMON_OBJS = io.o trie.o word.o
ENCODE_OBJS = encode.o $(COMMON_OBJS)
DECODE_OBJS = decode.o $(COMMON_OBJS)

all: encode decode

encode: $(ENCODE_OBJS)
	$(CC) $(CFLAGS) -o encode $(ENCODE_OBJS) -lm 
decode: $(DECODE_OBJS)
	$(CC) $(CFLAGS) -o decode $(DECODE_OBJS) -lm
%.o : %.c
	$(CC) $(CFLAGS) -c $<
clean:
	rm -f encode decode $(ENCODE_OBJS) $(DECODE_OBJS)
format:
	clang-format -i -style=file *.[ch]
