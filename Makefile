SRC_DEMO = ./test/tknet_demo.c
BIN_TKNET = ./bin/libtknet
BIN_DEMO = ./bin/tknet_demo

SRC = $(wildcard *.c)
OBJ = $(patsubst %.c, %.o, $(SRC)) 
OBJ_DEMO = $(patsubst %.c, %.o, $(SRC_DEMO))

LIB = #-lmcheck
CFLAG += -Wall -Werror=return-type -std=c89

CC = @echo "gcc $^";gcc
AR = @echo "create archive $@"; ar

all: env $(BIN_TKNET) $(BIN_DEMO)

env: 
	@ test -d bin || mkdir -p bin

$(BIN_TKNET): $(OBJ)
	$(AR) -rc $@ $^ 

$(BIN_DEMO): $(OBJ_DEMO) $(BIN_TKNET)
	$(CC) -o $@ $^ $(LIB)

%.o: %.c
	$(CC) $(CFLAG) -c $^ -I ./inc -o $@

test_%: env $(BIN_TKNET)
	$(CC) $(CFLAG) -c ./test/$@.c -I ./inc -o ./test/$@.o
	$(CC) -o ./bin/$@ ./test/$@.o $(BIN_TKNET) $(LIB)

clean:
	@ find . -name "*.o" -o -name "*.swp" | xargs rm -f
	@ rm -rf bin

push: clean
	git add .
	git commit
	git push origin re-design
