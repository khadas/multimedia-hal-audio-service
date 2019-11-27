COMMON_OBJS=src/CircularBuffer.o

PROTO_SRCS=src/audio_service.grpc.pb.cc src/audio_service.pb.cc
PROTO_OBJS+=$(PROTO_SRCS:.cc=.o)

SERVER_OBJS=src/audio_server.o src/audio_if.o
CLIENT_OBJS=src/audio_client.o src/audio_if_client.o

SERVER_OBJS+=$(COMMON_OBJS) $(PROTO_OBJS)
CLIENT_OBJS+=$(COMMON_OBJS) $(PROTO_OBJS)

TEST_PCM_OBJS=src/test.o
TEST_DOLBY_OBJS=src/test_ac3.o

PROTOC=$(HOST_DIR)/bin/protoc
PROTOC_INC=$(HOST_DIR)/include
GRPC_CPP_PLUGIN_PATH=$(HOST_DIR)/bin/grpc_cpp_plugin

CFLAGS+=-fPIC -O2 -I$(PROTOC_INC) -I./include -I. -I./src
CXXFLAGS+=-std=c++14
LDFLAGS+=-lgrpc++_unsecure -lprotobuf -lboost_system -llog -ldl -lrt -lpthread -lstdc++

%.grpc.pb.cc: %.proto
	$(PROTOC) -I=. -I=$(PROTOC_INC) --grpc_out=. --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH) $<

%.pb.cc: %.proto
	$(PROTOC) -I=. -I=$(PROTOC_INC) --cpp_out=. --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH) $<

%.o: %.cpp
	$(CC) -c $(CFLAGS) $(CXXFLAGS) -o $@ $<

%.o: %.cc
	$(CC) -c $(CFLAGS) $(CXXFLAGS) -o $@ $<

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

all: audio_server libaudio_client.so audio_client_test audio_client_test_ac3

audio_server: $(SERVER_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

libaudio_client.so: $(CLIENT_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -o $@ $^

audio_client_test: $(TEST_PCM_OBJS) libaudio_client.so
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

audio_client_test_ac3: $(TEST_DOLBY_OBJS) libaudio_client.so
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

.PHONY: install
install:
	install -m 755 -D audio_server -t $(TARGET_DIR)/usr/bin/
	install -m 755 -D audio_client_test -t $(TARGET_DIR)/usr/bin/
	install -m 755 -D audio_client_test_ac3 $(TARGET_DIR)/usr/bin/
	install -m 644 -D libaudio_client.so -t $(TARGET_DIR)/usr/lib/
	install -m 644 -D include/audio_if_client.h -t $(STAGING_DIR)/usr/include
	for f in $(@D)/include/hardware/*.h; do \
		install -m 644 -D $${f} -t $(STAGING_DIR)/usr/include/hardware; \
	done
	for f in $(@D)/include/system/*.h; do \
		install -m 644 -D $${f} -t $(STAGING_DIR)/usr/include/system; \
	done

.PHONY: clean
clean:
	rm -f audio_server
	rm -f audio_client_test
	rm -f audio_client_test_ac3
	rm -rf $(STAGING_DIR)/usr/include/hardware
	rm -rf $(STAGING_DIR)/usr/include/system
	rm -f libaudio_client.so
	rm -f $(TARGET_DIR)/usr/bin/audio_server
	rm -f $(TARGET_DIR)/usr/bin/audio_client_test
	rm -f $(TARGET_DIR)/usr/bin/audio_client_test_ac3
	rm -f $(TARGET_DIR)/usr/lib/libaudio_client.so
	rm -f $(STAGING_DIR)/usr/lib/libaudio_client.so
	rm -f $(STAGING_DIR)/usr/include/audio_if_client.h

