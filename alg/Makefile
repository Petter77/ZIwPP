CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Iinclude
TARGET   := main
BUILD    := build

SRCS := $(wildcard src/*.cpp)
OBJS := $(SRCS:src/%.cpp=$(BUILD)/%.o)
DEPS := $(OBJS:.o=.d)

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@

$(BUILD)/%.o: src/%.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILD):
	mkdir -p $(BUILD)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD) $(TARGET)

-include $(DEPS)
