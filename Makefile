CC = c99

OUT = out
OBJ = obj

LIBS = libelf libbsd
CFLAGS = $(shell pkg-config --cflags $(LIBS)) -DLIBBSD_OPENBSD_VIS=1
LDFLAGS = $(shell pkg-config --libs $(LIBS))

all: $(OUT) $(OBJ) $(OUT)/phdr $(OUT)/shdr $(OUT)/write $(OUT)/prog

$(OUT)/phdr: $(OBJ)/phdr.o
	$(CC) $(LDFLAGS) -o $@ $^

$(OUT)/shdr: $(OBJ)/shdr.o
	$(CC) $(LDFLAGS) -o $@ $^

$(OUT)/write: $(OBJ)/write.o
	$(CC) $(LDFLAGS) -o $@ $^

$(OUT)/prog: $(OBJ)/prog.o
	$(CC) $(LDFLAGS) -o $@ $^

$(OBJ)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(OUT) $(OBJ):
	mkdir -p $@

clean:
	-rm -r $(OBJ) $(OUT)
