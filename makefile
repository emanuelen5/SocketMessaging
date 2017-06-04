.PHONY: all clean
all:
	cd server && $(MAKE)

clean:
	cd server && $(MAKE) clean
