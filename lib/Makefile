SOURCES := aathread.c aascheduler.c aaqueue.c
TARGET := libaathread.a
OBJS := $(SOURCES:.c=.o)

CFLAGS := -Wall -Wextra -ggdb3 -O0

ARFLAGS := $(ARFLAGS)U

all: $(TARGET)

$(TARGET): $(TARGET)($(OBJS))

clean:
	rm -f $(TARGET) $(SOURCES:.c=.o) $(SOURCES:.c=.d) $(addsuffix .*, $(SOURCES:.c=.d))

%.d: %.c
	@set -e; rm -f $@; \
	$(CC) -M $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$


-include $(SOURCES:.c=.d)
