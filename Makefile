CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread
TARGET   = server
SRCS     = main.cpp server.cpp client_handler.cpp \
           http_request.cpp http_response.cpp \
           router.cpp mime_types.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)
