srcs:=$(wildcard *.cpp)
objects:=$(patsubst %.cpp, %.o, $(srcs))

usbloader: $(objects)
	$(CXX) -g -o $@ $^

clean:
	rm -rf usbloader *.o