#Set the appropriate path to the SDK headers
SDK_INSTALL_PATH=/usr

OBJS = SdkWrap.o main.o
CXX = g++
CXXFLAGS = -I$(SDK_INSTALL_PATH)/include/parallels-virtualization-sdk -DDYN_API_WRAP -ldl
LDFLAGS = -ldl

# Set the current folder name
TARGET = firstproject

all : $(TARGET)
$(TARGET) : $(OBJS)
	$(CXX) -o $@ $(LDFLAGS) $(OBJS)

main.o : main.cpp
	$(CXX) -c -o $@ $(CXXFLAGS) main.cpp

SdkWrap.o : $(SDK_INSTALL_PATH)/share/parallels-virtualization-sdk/helpers/SdkWrap/SdkWrap.cpp 
	$(CXX) -c -o $@ $(CXXFLAGS) $(SDK_INSTALL_PATH)/share/parallels-virtualization-sdk/helpers/SdkWrap/SdkWrap.cpp -DDYN_API_WRAP -ldl

clean:
	@rm -f $(TARGET) $(OBJS)
.PHONY : all clean

