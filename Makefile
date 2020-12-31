PROTO_SRCS=src/audio_service.grpc.pb.cc src/audio_service.pb.cc
PROTO_OBJS+=$(PROTO_SRCS:.cc=.o)

SERVER_OBJS=src/audio_server.o src/audio_if.o
CLIENT_OBJS=src/audio_client.o src/audio_if_client.o

SERVER_OBJS+=$(COMMON_OBJS) $(PROTO_OBJS)
CLIENT_OBJS+=$(COMMON_OBJS) $(PROTO_OBJS)

TEST_PCM_OBJS=src/test.o
TEST_DOLBY_OBJS=src/test_ac3.o
TEST_HALPLAY_OBJS=src/halplay.o
TEST_MS12_OBJS=src/dap_setting.o
TEST_SPEAKER_DELAY_OBJS=src/speaker_delay.o
TEST_DIGITAL_MODE_OBJS=src/digital_mode.o
TEST_ARC_TEST_OBJS=src/test_arc.o
TEST_START_ARC_OBJS=src/start_arc.o
TEST_HAL_PARAM_OBJS=src/hal_param.o
TEST_HAL_DUMP_OBJS=src/hal_dump.o
TEST_HAL_PATCH_OBJS=src/hal_patch.o
TEST_MASTER_VOL_OBJS=src/master_vol.o
EFFECT_TOOL_OBJS=src/effect_tool.o

PROTOC=$(HOST_DIR)/bin/protoc
PROTOC_INC=$(HOST_DIR)/include
GRPC_CPP_PLUGIN_PATH=$(HOST_DIR)/bin/grpc_cpp_plugin

CFLAGS+=-fPIC -O2 -I$(PROTOC_INC) -I./include -I. -I./src
CXXFLAGS+=-std=c++14
SC_LDFLAGS+=-Wl,--no-as-needed -lgrpc++_unsecure -lprotobuf -lboost_system -lamaudioutils -llog -ldl -lrt -lpthread -lstdc++ -pthread
LDFLAGS+= -Wl,--no-as-needed -llog -ldl -lrt -lpthread -lstdc++ -pthread

%.grpc.pb.cc: %.proto
	$(PROTOC) -I=. -I=$(PROTOC_INC) --grpc_out=. --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH) $<

%.pb.h: %.proto
	$(PROTOC) -I=. -I=$(PROTOC_INC) --cpp_out=. --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH) $<

%.pb.cc: %.proto
	$(PROTOC) -I=. -I=$(PROTOC_INC) --cpp_out=. --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH) $<

%.o: %.cpp
	$(CC) -c $(CFLAGS) $(CXXFLAGS) -o $@ $<

%.o: %.cc
	$(CC) -c $(CFLAGS) $(CXXFLAGS) -o $@ $<

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

src/audio_service.grpc.pb.cc: src/audio_service.pb.h
src/audio_server.cpp: src/audio_service.pb.h src/audio_service.grpc.pb.cc
src/audio_client.cpp: src/audio_service.pb.h src/audio_service.grpc.pb.cc
src/audio_if_client.cpp: src/audio_service.pb.h src/audio_service.grpc.pb.cc

all: audio_server libaudio_client.so audio_client_test audio_client_test_ac3 halplay dap_setting speaker_delay digital_mode test_arc start_arc hal_param hal_dump hal_patch master_vol effect_tool

audio_server: $(SERVER_OBJS)
	$(CC) $(CFLAGS) $(SC_LDFLAGS) -o $@ $^

libaudio_client.so: $(CLIENT_OBJS)
	$(CC) $(CFLAGS) $(SC_LDFLAGS) -shared -o $@ $^

audio_client_test: $(TEST_PCM_OBJS) libaudio_client.so
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

audio_client_test_ac3: $(TEST_DOLBY_OBJS) libaudio_client.so
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

halplay: $(TEST_HALPLAY_OBJS) libaudio_client.so
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

dap_setting: $(TEST_MS12_OBJS) libaudio_client.so
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

speaker_delay: $(TEST_SPEAKER_DELAY_OBJS) libaudio_client.so
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

digital_mode: $(TEST_DIGITAL_MODE_OBJS) libaudio_client.so
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

test_arc: $(TEST_ARC_TEST_OBJS) libaudio_client.so
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

start_arc: $(TEST_START_ARC_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

hal_param: $(TEST_HAL_PARAM_OBJS) libaudio_client.so
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

hal_dump: $(TEST_HAL_DUMP_OBJS) libaudio_client.so
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

hal_patch: $(TEST_HAL_PATCH_OBJS) libaudio_client.so
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

master_vol: $(TEST_MASTER_VOL_OBJS) libaudio_client.so
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

effect_tool: $(EFFECT_TOOL_OBJS) libaudio_client.so
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

.PHONY: install
install:
	install -m 755 -D audio_server -t $(TARGET_DIR)/usr/bin/
	install -m 755 -D audio_client_test -t $(TARGET_DIR)/usr/bin/
	install -m 755 -D audio_client_test_ac3 $(TARGET_DIR)/usr/bin/
	install -m 755 -D halplay $(TARGET_DIR)/usr/bin/
	install -m 755 -D dap_setting $(TARGET_DIR)/usr/bin/
	install -m 755 -D speaker_delay $(TARGET_DIR)/usr/bin/
	install -m 755 -D digital_mode $(TARGET_DIR)/usr/bin/
	install -m 755 -D test_arc $(TARGET_DIR)/usr/bin/
	install -m 755 -D start_arc $(TARGET_DIR)/usr/bin/
	install -m 755 -D hal_param $(TARGET_DIR)/usr/bin/
	install -m 755 -D hal_dump $(TARGET_DIR)/usr/bin/
	install -m 755 -D hal_patch $(TARGET_DIR)/usr/bin/
	install -m 755 -D master_vol $(TARGET_DIR)/usr/bin/
	install -m 755 -D effect_tool $(TARGET_DIR)/usr/bin/
	install -m 644 -D libaudio_client.so -t $(TARGET_DIR)/usr/lib/
	install -m 644 -D libaudio_client.so -t $(STAGING_DIR)/usr/lib/
	install -m 644 -D include/audio_if_client.h -t $(STAGING_DIR)/usr/include
	install -m 644 -D include/audio_if.h -t $(STAGING_DIR)/usr/include
	install -m 644 -D include/audio_effect_if.h -t $(STAGING_DIR)/usr/include
	install -m 644 -D include/audio_effect_params.h -t $(STAGING_DIR)/usr/include
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
	rm -f halplay
	rm -f test_arc
	rm -f start_arc
	rm -f hal_param
	rm -f hal_dump
	rm -f hal_patch
	rm -f master_vol
	rm -f effect_tool
	rm -rf $(STAGING_DIR)/usr/include/hardware
	rm -rf $(STAGING_DIR)/usr/include/system
	rm -f libaudio_client.so
	rm -f $(TARGET_DIR)/usr/bin/audio_server
	rm -f $(TARGET_DIR)/usr/bin/audio_client_test
	rm -f $(TARGET_DIR)/usr/bin/audio_client_test_ac3
	rm -f $(TARGET_DIR)/usr/bin/halplay
	rm -f $(TARGET_DIR)/usr/bin/speaker_delay
	rm -f $(TARGET_DIR)/usr/bin/digital_mode
	rm -f $(TARGET_DIR)/usr/bin/test_arc
	rm -f $(TARGET_DIR)/usr/bin/start_arc
	rm -f $(TARGET_DIR)/usr/bin/hal_param
	rm -f $(TARGET_DIR)/usr/bin/hal_dump
	rm -f $(TARGET_DIR)/usr/bin/hal_patch
	rm -f $(TARGET_DIR)/usr/bin/master_vol
	rm -f $(TARGET_DIR)/usr/bin/effect_tool
	rm -f $(TARGET_DIR)/usr/lib/libaudio_client.so
	rm -f $(STAGING_DIR)/usr/lib/libaudio_client.so
	rm -f $(STAGING_DIR)/usr/include/audio_if_client.h
	rm -f $(STAGING_DIR)/usr/include/audio_effect_if.h
	rm -f $(STAGING_DIR)/usr/include/audio_effect_params.h

