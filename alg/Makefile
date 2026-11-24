CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -Iinclude
LDFLAGS  := -pthread
TARGET   := main
GENERATOR := generator
BUILD    := build

SRCS := $(shell find src -name '*.cpp')
OBJS := $(SRCS:src/%.cpp=$(BUILD)/%.o)
DEPS := $(OBJS:.o=.d)

.PHONY: all run clean

all: $(TARGET) $(GENERATOR)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Generator jest osobnym programem: tylko jego źródło i wczytywanie instancji.
$(GENERATOR): tools/generator.cpp src/instance.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD) $(TARGET) $(GENERATOR)

-include $(DEPS)
