CC = clang
CFLAGS = -std=gnu11 -O2 -Wall -Wextra -Werror -pedantic -Wno-unused-result
MUMSH_SRC = *.c
MUMSH = mumsh
MUMSHMC = mumsh_memory_check
MUMSHMC_FLAGS = -fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined
	-fsanitize=integer
.PHONY: clean

all: $(MUMSH) $(MUMSHMC)

$(MUMSH): $(MUMSH_SRC)
	$(CC) $(CFLAGS) -o $(MUMSH) $(MUMSH_SRC)

$(MUMSHMC) : $(MUMSH_SRC)
	$(CC) $(CFLAGS) -o $(MUMSH) $(MUMSH_SRC)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

Clean:
	$(RM) *.o *.a *~ $(MUMSH) $(MUMSHMC)

