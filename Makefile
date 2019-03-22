OBJS   = usb.o gmmk.o main.o
TARGET = gmmkctl
CFLAGS = `pkg-config libusb-1.0 --cflags` -Wall -Wextra -ggdb -Og
LDFLAGS = `pkg-config libusb-1.0 --libs`

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS)

all: $(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: clean
