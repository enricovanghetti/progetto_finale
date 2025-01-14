
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -pedantic
TARGET = home_system

INCLUDE_DIR = include
SRC_DIR = src

SRCS = $(SRC_DIR)/main_program.cpp $(SRC_DIR)/device_implementation.cpp $(SRC_DIR)/DeviceManager.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp $(INCLUDE_DIR)/*.h
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
