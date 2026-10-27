CXX      := c++
CXXFLAGS := -std=c++17 -Wall -Wextra -g -MMD -MP

TARGET := build/serv2

SRCS := main.cpp \
        server/async.cpp \
        utils/evals.cpp \
        utils/resp.cpp \
        utils/store.cpp \
        utils/eviction.cpp \
        utils/aof.cpp

OBJS := $(SRCS:.cpp=.o)
DEPS := $(OBJS:.o=.d)

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET)

-include $(DEPS)

.PHONY: all clean
