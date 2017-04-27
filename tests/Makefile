SOURCES := three-threads.c
TARGET := $(SOURCES:.c=)
#OBJS := $(SOURCES:.c=.o)

CFLAGS := -Wall -Wextra -ggdb3 -O0
LDFLAGS := -L../lib
LDLIBS := -laathread -lrt

vpath lib%.a ../lib

all: $(TARGET)

$(TARGET): -laathread

clean:
	rm -f $(TARGET) $(SOURCES:.c=.o) $(SOURCES:.c=.d) $(addsuffix .*, $(SOURCES:.c=.d))

%.d: %.c
	@set -e; rm -f $@; \
	$(CC) -M $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

-include $(SOURCES:.c=.d)
