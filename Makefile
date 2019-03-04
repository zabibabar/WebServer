TARGETS = Weblite Server Knock_Catcher

CC_C = $(CROSS_TOOL)gcc

CFLAGS = -lpthread -lrt -lm

all: clean $(TARGETS)

$(TARGETS):
	$(CC_C) $(CFLAGS) $@.c -o $@

clean:
	rm -f $(TARGETS)