// Copyright 2023 Amlogic, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <functional>

#include "audio_service_binder.h"
#include "service_death_recipient.h"

#define CHKP_AND_RET(p, r) do { if (!(p)) return (r); } while (0)

std::mutex AudioServiceBinder::map_in_mutex_;
std::mutex AudioServiceBinder::map_out_mutex_;
std::mutex AudioServiceBinder::gc_map_mutex_;

AudioServiceBinder* AudioServiceBinder::mInstance = nullptr;

AudioServiceBinder* AudioServiceBinder::GetInstance() {
    if (mInstance == nullptr) {
        mInstance = new AudioServiceBinder();
    }

    return mInstance;
}

AudioServiceBinder::AudioServiceBinder() {
    if (audio_hw_load_interface(&dev_) == 0) {
        if (dev_) {
            std::cout<< PROCESS_NAME_LOG << " Got audio hal interface successfully." << std::endl;
            effect_ = (audio_effect_t*) dev_->common.reserved[0];
        }
    }
}

AudioServiceBinder::~AudioServiceBinder() {
    for (std::unordered_set<audio_patch_handle_t>::const_iterator it = audioPatchHandles.begin(); it != audioPatchHandles.end(); ++it) {
        audio_patch_handle_t patch = *it;
        Device_release_audio_patch(patch);
    }

    audioPatchHandles.clear();

    if (dev_) {
        if (effect_) {
            effect_ = nullptr;
        }
        audio_hw_unload_interface(dev_);
        dev_ = nullptr;
    }
}

void AudioServiceBinder::on_client_turned_off(int clientPid) {
    std::lock_guard<std::mutex> lock(gc_map_mutex_);

    std::map<int, gc_stream_map_t>::iterator gc_map_it = gc_stream_map_.find(clientPid);
    if (gc_map_it == gc_stream_map_.end()) {
        std::cout << PROCESS_NAME_LOG << " Could not find entry with pid " << clientPid << " in garbage collection stream map." << std::endl;
        return;
    }

    gc_stream_map_t gc_stream_map_pair = gc_map_it->second;

    ::android::sp<ServiceDeathRecipient> serviceDeathRecipient = gc_stream_map_pair.first;
    if (serviceDeathRecipient != nullptr) serviceDeathRecipient.clear();

    std::map<const std::string, stream_t>& streamNamesInfo = gc_stream_map_pair.second;

    for (std::map<const std::string, stream_t>::const_iterator streamNamesInfoItr = streamNamesInfo.begin(); streamNamesInfoItr != streamNamesInfo.end(); ++streamNamesInfoItr) {
        const std::string streamName = streamNamesInfoItr->first;
        stream_t streamType = streamNamesInfoItr->second;

        if (streamType == STREAM_OUT) {
            streamout_gc_(streamName);
        } else {
            streamin_gc_(streamName);
        }
    }

    streamNamesInfo.clear();

    gc_stream_map_.erase(gc_map_it);
}

int AudioServiceBinder::Device_common_close() {
    CHKP_AND_RET(dev_, -1);
    return dev_->common.close(&dev_->common);
}

int AudioServiceBinder::Device_init_check() {
    CHKP_AND_RET(dev_, -1);
    return dev_->init_check(dev_);
}

int AudioServiceBinder::Device_set_voice_volume(float volume) {
    CHKP_AND_RET(dev_, -1);
    return dev_->set_voice_volume(dev_, volume);
}

int AudioServiceBinder::Device_set_master_volume(float volume) {
    CHKP_AND_RET(dev_, -1);
    return dev_->set_master_volume(dev_, volume);
}

int AudioServiceBinder::Device_get_master_volume(float& masterVolume) {
    CHKP_AND_RET(dev_, -1);
    float vol = 0.0;
    int ret = dev_->get_master_volume(dev_, &vol);
    masterVolume = vol;
    return ret;
}

int AudioServiceBinder::Device_set_mode(audio_mode_t mode) {
    CHKP_AND_RET(dev_, -1);
    return dev_->set_mode(dev_, mode);
}

int AudioServiceBinder::Device_set_mic_mute(bool state) {
    CHKP_AND_RET(dev_, -1);
    return dev_->set_mic_mute(dev_, state);
}

int AudioServiceBinder::Device_get_mic_mute(bool& state) {
    CHKP_AND_RET(dev_, -1);
    return dev_->get_mic_mute(dev_, &state);
}

int AudioServiceBinder::Device_set_parameters(const char* kv_pairs) {
    CHKP_AND_RET(dev_, -1);
    return dev_->set_parameters(dev_, kv_pairs);
}

char* AudioServiceBinder::Device_get_parameters(const char* keys) {
    CHKP_AND_RET(dev_, nullptr);
    return dev_->get_parameters(dev_, keys);
}

int AudioServiceBinder::Device_get_input_buffer_size(const struct audio_config& config) {
    CHKP_AND_RET(dev_, -1);
    return dev_->get_input_buffer_size(dev_, &config);
}

int AudioServiceBinder::Device_open_output_stream(::android::sp<::android::IBinder> audioClientBinder,
                                            int seq,
                                            audio_io_handle_t handle,
                                            audio_devices_t devices,
                                            audio_output_flags_t flags,
                                            struct audio_config& config,
                                            const char* address,
                                            int& clientId) {
    CHKP_AND_RET(dev_, -1);

    int client_id = ::android::IPCThreadState::self()->getCallingPid();
    if (client_id < 0) {
        clientId = -1;
        return -1;
    }

    char buf[32];
    snprintf(buf, sizeof(buf), "%d-%d", client_id, seq);
    std::string stream_id(buf);

    struct audio_stream_out* stream = nullptr;

    int ret = dev_->open_output_stream(dev_, handle, devices, flags, &config, &stream, address);
    clientId = client_id;

    if (stream) {
        update_gc_stream_map(audioClientBinder, client_id, stream_id, STREAM_OUT);

        const char* stream_id_c_str = stream_id.c_str();

        void* streamoutShm;

        umask(STREAM_OBJECT_UMASK);
        int fd = posixOpenFileSetDataSizeAndMapData(PROCESS_NAME_LOG, OUTPUT_STREAM_LABEL, stream_id_c_str, O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO, KSHAREDBUFFERSIZE,
                                            streamoutShm, PROT_READ | PROT_WRITE, MAP_SHARED);
        if (fd == -1) { return -1; }

        std::cout << PROCESS_NAME_LOG << " open " << OUTPUT_STREAM_LABEL << " " << stream_id_c_str << std::endl;

        {
            std::lock_guard<std::mutex> lock(map_out_mutex_);
            streamout_map_.insert(std::pair<const std::string, streamout_map_t>(stream_id, streamout_map_t(fd, streamout_data_t(streamoutShm, stream))));
        }
    }

    return ret;
}

void AudioServiceBinder::Device_close_output_stream(const char* name) {
    const std::string nameStr(name);
    {
        std::lock_guard<std::mutex> lock_out(map_out_mutex_);

        std::map<const std::string, streamout_map_t>::iterator it = streamout_map_.find(nameStr);
        if (it == streamout_map_.end()) return;

        int fd = it->second.first;
        posixUnlinkNameAndCloseFile(PROCESS_NAME_LOG, OUTPUT_STREAM_LABEL, name, fd);

        if (dev_) {
            std::cout << PROCESS_NAME_LOG << " close " << OUTPUT_STREAM_LABEL << " " << name << std::endl;
            dev_->close_output_stream(dev_, it->second.second.second);
        }

        streamout_map_.erase(it);
    }

    int client_id = ::android::IPCThreadState::self()->getCallingPid();
    remove_stream_info_from_gc_stream_map(client_id, nameStr);
}

int AudioServiceBinder::Device_open_input_stream(::android::sp<::android::IBinder> audioClientBinder,
                                            int seq,
                                            audio_io_handle_t handle,
                                            audio_devices_t devices,
                                            struct audio_config& config,
                                            audio_input_flags_t flags,
                                            const char* address,
                                            audio_source_t source,
                                            int& clientId) {
    CHKP_AND_RET(dev_, -1);

    int client_id = ::android::IPCThreadState::self()->getCallingPid();
    if (client_id < 0) {
        clientId = -1;
        return -1;
    }
    char buf[32];
    snprintf(buf, sizeof(buf), "%d-%d", client_id, seq);
    std::string stream_id(buf);

    struct audio_stream_in* stream = nullptr;

    int ret = dev_->open_input_stream(dev_, handle, devices, &config, &stream, flags, address, source);
    clientId = client_id;

    if (stream) {
        update_gc_stream_map(audioClientBinder, client_id, stream_id, STREAM_IN);

        const char* stream_id_c_str = stream_id.c_str();

        umask(STREAM_OBJECT_UMASK);

        void* streaminShm; int fd;
        fd = posixOpenFileSetDataSizeAndMapData(PROCESS_NAME_LOG, INPUT_STREAM_LABEL, stream_id_c_str, O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO, KSHAREDBUFFERSIZE,
                                            streaminShm, PROT_READ | PROT_WRITE, MAP_SHARED);
        if (fd == -1) { return -1; }

        std::cout << PROCESS_NAME_LOG << " open " << INPUT_STREAM_LABEL << " " << stream_id_c_str << std::endl;

        {
            std::lock_guard<std::mutex> lock(map_in_mutex_);
            streamin_map_.insert(std::pair<const std::string, streamin_map_t>(stream_id, streamin_map_t(fd, streamin_data_t(streaminShm, stream))));
        }
    }

    return ret;
}

void AudioServiceBinder::Device_close_input_stream(const char* name) {
    const std::string nameStr(name);
    {
        std::lock_guard<std::mutex> lock(map_in_mutex_);

        std::map<const std::string, streamin_map_t>::iterator it = streamin_map_.find(nameStr);
        if (it == streamin_map_.end()) return;

        int fd = it->second.first;
        posixUnlinkNameAndCloseFile(PROCESS_NAME_LOG, INPUT_STREAM_LABEL, name, fd);

        if (dev_) {
            std::cout << PROCESS_NAME_LOG << " close " << INPUT_STREAM_LABEL << " " << name << std::endl;
            dev_->close_input_stream(dev_, it->second.second.second);
        }

        streamin_map_.erase(it);
    }

    int client_id = ::android::IPCThreadState::self()->getCallingPid();
    remove_stream_info_from_gc_stream_map(client_id, nameStr);
}

int AudioServiceBinder::Device_dump(std::string& deviceDump) {
    int ret = -1;

    CHKP_AND_RET(dev_, ret);

    char* param = dev_->dump(dev_, 0);
    ret = param ? 0 : -1;

    deviceDump = std::string(param);

    // param is heap allocated and need free
    free(param);

    return ret;
}

int AudioServiceBinder::Device_set_master_mute(bool mute) {
    CHKP_AND_RET(dev_, -1);
    return dev_->set_master_mute(dev_, mute);
}

int AudioServiceBinder::Device_get_master_mute(bool& mute) {
    CHKP_AND_RET(dev_, -1);
    return dev_->get_master_mute(dev_, &mute);
}

int AudioServiceBinder::Device_create_audio_patch(unsigned int num_sources,
                                                const struct audio_port_config* sources,
                                                unsigned int num_sinks,
                                                const struct audio_port_config* sinks,
                                                audio_patch_handle_t& handle) {
    CHKP_AND_RET(dev_, -1);

    handle = (audio_patch_handle_t) (-1);

    int ret = dev_->create_audio_patch(dev_, num_sources, sources, num_sinks, sinks, &handle);
    if (!ret) {
        audioPatchHandles.insert(handle);
    }
    return ret;
}

int AudioServiceBinder::Device_release_audio_patch(audio_patch_handle_t handle) {
    CHKP_AND_RET(dev_, -1);

    int ret = dev_->release_audio_patch(dev_, handle);
    if (!ret) {
        audioPatchHandles.erase(handle);
    }
    return ret;
}

int AudioServiceBinder::Device_set_audio_port_config(const struct audio_port_config& config) {
    CHKP_AND_RET(dev_, -1);
    return dev_->set_audio_port_config(dev_, &config);
}

uint32_t AudioServiceBinder::Stream_get_sample_rate(const char* name) {
    CHKP_AND_RET(dev_, -1);

    struct audio_stream* stream = find_stream(std::string(name), streamout_map_, streamin_map_);
    CHKP_AND_RET(stream, -1);

    return stream->get_sample_rate(stream);
}

size_t AudioServiceBinder::Stream_get_buffer_size(const char* name) {
    CHKP_AND_RET(dev_, -1);

    struct audio_stream* stream = find_stream(std::string(name), streamout_map_, streamin_map_);
    CHKP_AND_RET(stream, -1);

    return stream->get_buffer_size(stream);
}

uint32_t AudioServiceBinder::Stream_get_channels(const char* name) {
    struct audio_stream* stream = find_stream(std::string(name), streamout_map_, streamin_map_);
    return (uint32_t) stream->get_channels(stream);
}

int AudioServiceBinder::Stream_get_format(const char* name) {
    struct audio_stream* stream = find_stream(std::string(name), streamout_map_, streamin_map_);
    return (uint32_t) stream->get_format(stream);
}

uint32_t AudioServiceBinder::Stream_standby(const char* name) {
    CHKP_AND_RET(dev_, -1);

    struct audio_stream* stream = find_stream(std::string(name), streamout_map_, streamin_map_);
    CHKP_AND_RET(stream, -1);

    return (uint32_t) stream->standby(stream);
}

uint32_t AudioServiceBinder::Stream_get_device(const char* name) {
    CHKP_AND_RET(dev_, -1);

    struct audio_stream* stream = find_stream(std::string(name), streamout_map_, streamin_map_);
    CHKP_AND_RET(stream, -1);

    return (uint32_t) stream->get_device(stream);
}

int AudioServiceBinder::Stream_set_parameters(const char* name, const char* kv_pairs) {
    CHKP_AND_RET(dev_, -1);

    struct audio_stream* stream = find_stream(std::string(name), streamout_map_, streamin_map_);
    CHKP_AND_RET(stream, -1);

    return stream->set_parameters(stream, kv_pairs);
}

char* AudioServiceBinder::Stream_get_parameters(const char* name, const char* keys) {
    CHKP_AND_RET(dev_, nullptr);

    struct audio_stream* stream = find_stream(std::string(name), streamout_map_, streamin_map_);
    CHKP_AND_RET(stream, nullptr);

    return stream->get_parameters(stream, keys);
}

uint32_t AudioServiceBinder::StreamOut_get_latency(const char* name) {
    CHKP_AND_RET(dev_, -1);

    struct audio_stream_out *stream = find_streamout(std::string(name), streamout_map_);
    CHKP_AND_RET(stream, -1);

    return stream->get_latency(stream);
}

int AudioServiceBinder::StreamOut_set_volume(const char* name, float left, float right) {
    CHKP_AND_RET(dev_, -1);

    struct audio_stream_out* stream = find_streamout(std::string(name), streamout_map_);
    CHKP_AND_RET(stream, -1);

    return stream->set_volume(stream, left, right);
}

ssize_t AudioServiceBinder::StreamOut_write(const char* name, size_t bytes) {
    CHKP_AND_RET(dev_, -1);

    void* streamoutShm; struct audio_stream_out* stream;
    if (getStreamShmAndStream<std::map<const std::string, streamout_map_t>, struct audio_stream_out*>(OUTPUT_STREAM_LABEL, name, streamout_map_, streamoutShm, KSHAREDBUFFERSIZE, stream) == -1) { return -1; }

    return stream->write(stream, streamoutShm, bytes);
}

int AudioServiceBinder::StreamOut_get_render_position(const char* name, uint32_t& dsp_frames) {
    CHKP_AND_RET(dev_, -1);

    struct audio_stream_out* stream = find_streamout(std::string(name), streamout_map_);
    CHKP_AND_RET(stream, -1);

    return stream->get_render_position(stream, &dsp_frames);
}

int AudioServiceBinder::StreamOut_get_next_write_timestamp(const char* name, int64_t& timestamp) {
    CHKP_AND_RET(dev_, -1);

    struct audio_stream_out* stream = find_streamout(std::string(name), streamout_map_);
    CHKP_AND_RET(stream, -1);

    timestamp = 0;
    return stream->get_next_write_timestamp(stream, &timestamp);
}

int AudioServiceBinder::StreamOut_pause(const char* name) {
    CHKP_AND_RET(dev_, -1);

    struct audio_stream_out* stream = find_streamout(std::string(name), streamout_map_);
    CHKP_AND_RET(stream, -1);

    return stream->pause(stream);
}

int AudioServiceBinder::StreamOut_resume(const char* name) {
    CHKP_AND_RET(dev_, -1);

    struct audio_stream_out* stream = find_streamout(std::string(name), streamout_map_);
    CHKP_AND_RET(stream, -1);

    return stream->resume(stream);
}

int AudioServiceBinder::StreamOut_flush(const char* name) {
    CHKP_AND_RET(dev_, -1);

    struct audio_stream_out* stream = find_streamout(std::string(name), streamout_map_);
    CHKP_AND_RET(stream, -1);

    return stream->flush(stream);
}

int AudioServiceBinder::StreamOut_get_presentation_position(const char* name, uint64_t& frames, struct timespec& timestamp) {
    CHKP_AND_RET(dev_, -1);

    struct audio_stream_out* stream = find_streamout(std::string(name), streamout_map_);
    CHKP_AND_RET(stream, -1);

    frames = 0;
    return stream->get_presentation_position(stream, &frames, &timestamp);
}

int AudioServiceBinder::StreamIn_set_gain(const char* name, float gain) {
    CHKP_AND_RET(dev_, -1);

    struct audio_stream_in* stream = find_streamin(std::string(name), streamin_map_);
    CHKP_AND_RET(stream, -1);

    return stream->set_gain(stream, gain);
}

int AudioServiceBinder::StreamIn_read(const char* name, size_t bytes) {
    CHKP_AND_RET(dev_, -1);

    int ret;
    void* streaminShm; struct audio_stream_in* stream;
    if (getStreamShmAndStream<std::map<const std::string, streamin_map_t>, struct audio_stream_in*>(INPUT_STREAM_LABEL, name, streamin_map_, streaminShm, KSHAREDBUFFERSIZE, stream) == -1) { return -1; }
    ret = stream->read(stream, streaminShm, std::min(bytes, (size_t) KSHAREDBUFFERSIZE));

    return ret;
}

uint32_t AudioServiceBinder::StreamIn_get_input_frames_lost(const char* name) {
    CHKP_AND_RET(dev_, -1);

    struct audio_stream_in* stream = find_streamin(std::string(name), streamin_map_);
    CHKP_AND_RET(stream, -1);

    return stream->get_input_frames_lost(stream);
}

int AudioServiceBinder::StreamIn_get_capture_position(const char* name, int64_t& frames, int64_t& time) {
    CHKP_AND_RET(dev_, -1);

    struct audio_stream_in* stream = find_streamin(std::string(name), streamin_map_);
    CHKP_AND_RET(stream, -1);

    frames = time = 0;
    return stream->get_capture_position(stream, &frames, &time);
}

int AudioServiceBinder::Service_ping(int& status_32) {
    status_32 = 0;
    return 0;
}

int AudioServiceBinder::Effect_set_parameters(aml_audio_effect_type_e type,
                                        uint32_t cmdSize,
                                        void* pCmdData,
                                        uint32_t replySize,
                                        uint32_t& pReplyData) {
    if (!dev_ || !effect_) return -1;

    pReplyData = 0;
    return effect_->set_parameters(type, cmdSize, pCmdData, &replySize, &pReplyData);
}

int AudioServiceBinder::Effect_get_parameters(aml_audio_effect_type_e type,
                            uint32_t cmdSize,
                            void* pCmdData,
                            uint32_t& replySize,
                            void* pReplyData) {
    if (!dev_ || !effect_) return -1;

    return effect_->get_parameters(type, cmdSize, pCmdData, &replySize, pReplyData);
}

void AudioServiceBinder::streamout_gc_(const std::string& name) {
    std::lock_guard<std::mutex> lock(map_out_mutex_);

    std::map<const std::string, streamout_map_t >::iterator it = streamout_map_.find(name);
    if (it != streamout_map_.end()) {
        int fd = it->second.first; const char* name = it->first.c_str();
        posixUnlinkNameAndCloseFile(PROCESS_NAME_LOG, OUTPUT_STREAM_LABEL, name, fd);

        if (dev_) {
            std::cout << PROCESS_NAME_LOG << " close " << OUTPUT_STREAM_LABEL << " " << name << std::endl;
            dev_->close_output_stream(dev_, it->second.second.second);
        }

        streamout_map_.erase(it);
    }
}

void AudioServiceBinder::streamin_gc_(const std::string& name) {
    std::lock_guard<std::mutex> lock(map_in_mutex_);

    std::map<const std::string, streamin_map_t >::iterator it = streamin_map_.find(name);
    if (it != streamin_map_.end()) {
        int fd = it->second.first; const char* name = it->first.c_str();
        posixUnlinkNameAndCloseFile(PROCESS_NAME_LOG, INPUT_STREAM_LABEL, name, fd);

        if (dev_) {
            std::cout << PROCESS_NAME_LOG << " close " << INPUT_STREAM_LABEL << " " << name << std::endl;
            dev_->close_input_stream(dev_, it->second.second.second);
        }

        streamin_map_.erase(name);
    }
}

struct audio_stream* AudioServiceBinder::find_stream(const std::string& name,
                                                    const std::map<const std::string, streamout_map_t>& map_out,
                                                    const std::map<const std::string, streamin_map_t>& map_in) {
    std::lock_guard<std::mutex> lock_out(map_out_mutex_);
    std::map<const std::string, streamout_map_t>::const_iterator it_out = map_out.find(name);
    if (it_out != map_out.end()) {
        return &it_out->second.second.second->common;
    }

    std::lock_guard<std::mutex> lock_in(map_in_mutex_);
    std::map<const std::string, streamin_map_t>::const_iterator it_in = map_in.find(name);
    if (it_in != map_in.end()) {
        return &it_in->second.second.second->common;
    }

    return nullptr;
}

struct audio_stream_out* AudioServiceBinder::find_streamout(const std::string& name, const std::map<const std::string, streamout_map_t>& map_out) {
    std::lock_guard<std::mutex> lock(map_out_mutex_);
    std::map<const std::string, streamout_map_t>::const_iterator it = map_out.find(name);
    if (it != map_out.end()) {
        return it->second.second.second;
    }

    return nullptr;
}

struct audio_stream_in* AudioServiceBinder::find_streamin(const std::string& name, const std::map<const std::string, streamin_map_t>& map_in) {
    std::lock_guard<std::mutex> lock(map_in_mutex_);
    std::map<const std::string, streamin_map_t>::const_iterator it = map_in.find(name);
    if (it != map_in.end()) {
        return it->second.second.second;
    }

    return nullptr;
}

void AudioServiceBinder::update_gc_stream_map(::android::sp<::android::IBinder> audioClientBinder, int client_id, const std::string& stream_id, stream_t streamType) {
    std::lock_guard<std::mutex> lock(gc_map_mutex_);
    std::map<int, gc_stream_map_t>::iterator it = gc_stream_map_.find(client_id);
    if (it == gc_stream_map_.end()) {
        ::android::sp<ServiceDeathRecipient> serviceDeathRecipient = new ServiceDeathRecipient(this, client_id);
        audioClientBinder->linkToDeath(serviceDeathRecipient);
        std::map<const std::string, stream_t> streamNamesInfo; streamNamesInfo.insert(std::pair<const std::string, stream_t>(stream_id, streamType));
        gc_stream_map_.insert(std::pair<int, gc_stream_map_t>(client_id, gc_stream_map_t(serviceDeathRecipient, streamNamesInfo)));
    } else {
        // service death recipient registered to client_id
        it->second.second.insert(std::pair<const std::string, stream_t>(stream_id, streamType));
    }
}

void AudioServiceBinder::remove_stream_info_from_gc_stream_map(int clientPid, const std::string& name) {
    std::lock_guard<std::mutex> lock_gc(gc_map_mutex_);

    std::map<int, gc_stream_map_t>::iterator it = gc_stream_map_.find(clientPid);
    if (it == gc_stream_map_.end()) return;

    it->second.second.erase(name);
}

template <typename stream_map_t, typename stream_t> int AudioServiceBinder::getStreamShmAndStream(const char* streamLabel,
                                                                                                const char* name,
                                                                                                stream_map_t& streamMap,
                                                                                                void*& shm,
                                                                                                size_t length,
                                                                                                stream_t& stream) {
    std::string nameStr(name);
    typename stream_map_t::iterator it = streamMap.find(nameStr);
    if (it == streamMap.end()) { return -1; }

    std::pair<void*, stream_t> streamData = it->second.second;

    shm = streamData.first;
    if (!shm) {
        std::cout << PROCESS_NAME_LOG << " Shared memory to " << streamLabel << " " << nameStr << " was not previously allocated on service. Allocating." << std::endl;

        umask(STREAM_OBJECT_UMASK);
        int fd = posixOpenFileSetDataSizeAndMapData(PROCESS_NAME_LOG, streamLabel, name, O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO, length,
                                            shm, PROT_READ | PROT_WRITE, MAP_SHARED);
        if (fd == -1) { return -1; }

        it->second.second.first = shm;
    }

    stream = streamData.second;
    CHKP_AND_RET(stream, -1);

    return 0;
}

void AudioServiceBinder::readAudioConfigFromParcel(struct audio_config& config, const ::android::Parcel& parcel) {
    uint32_t sample_rate_channel_mask_frame_count[3];
    parcel.read(sample_rate_channel_mask_frame_count, sizeof(sample_rate_channel_mask_frame_count));
    config.sample_rate = sample_rate_channel_mask_frame_count[0];
    config.channel_mask = (audio_channel_mask_t) sample_rate_channel_mask_frame_count[1];
    config.frame_count = sample_rate_channel_mask_frame_count[2];
    config.format = (audio_format_t) parcel.readInt32();
}

void AudioServiceBinder::readAudioPortConfigFromParcel(struct audio_port_config& config, const ::android::Parcel& parcel) {
    config.id = parcel.readInt32();
    config.role = (audio_port_role_t) parcel.readInt32();

    audio_port_type_t type = (audio_port_type_t) parcel.readInt32();
    config.type = type;

    config.config_mask = parcel.readUint32();
    config.sample_rate = parcel.readUint32();
    config.channel_mask = parcel.readUint32();
    config.format = (audio_format_t) parcel.readInt32();

    struct audio_gain_config* gain = &config.gain;
    gain->index = parcel.readInt32();
    gain->mode = (audio_gain_mode_t) parcel.readUint32();
    gain->channel_mask = (audio_channel_mask_t) parcel.readInt32();

    parcel.read(gain->values, sizeof(gain->values));

    gain->ramp_duration_ms = parcel.readUint32();

    if (type == AUDIO_PORT_TYPE_DEVICE) {
        struct audio_port_config_device_ext* device = &config.ext.device;
        device->hw_module = (audio_module_handle_t) parcel.readInt32();
        device->type = (audio_devices_t) parcel.readUint32();
        strncpy(device->address, parcel.readCString(), AUDIO_DEVICE_MAX_ADDRESS_LEN);
    } else if (type == AUDIO_PORT_TYPE_MIX) {
        struct audio_port_config_mix_ext* mix = &config.ext.mix;
        mix->hw_module = (audio_module_handle_t) parcel.readInt32();
        mix->handle = (audio_io_handle_t) parcel.readInt32();
        mix->usecase.stream = (audio_stream_type_t) parcel.readInt32();
    } else if (type == AUDIO_PORT_TYPE_SESSION) {
        config.ext.session.session = (audio_session_t) parcel.readInt32();
    }
}

void AudioServiceBinder::readAudioPortConfigsFromParcel(unsigned int numConfigs, struct audio_port_config* configs, const ::android::Parcel& parcel) {
    struct audio_port_config* config = configs;
    for (unsigned int i = 0; i < numConfigs; i++, config++) { readAudioPortConfigFromParcel(*config, parcel); }
}

::android::status_t AudioServiceBinder::onTransact(uint32_t code, const ::android::Parcel& data, ::android::Parcel* reply, uint32_t flags) {
    switch (code) {
        case CMD_AS_DCC: {
            int ret = Device_common_close();
            reply->writeInt32(ret);
            break;
        }
        case CMD_AS_DIC: {
            int ret = Device_init_check();
            reply->writeInt32(ret);
            break;
        }
        case CMD_AS_DSVV: {
            int ret = Device_set_voice_volume(data.readFloat());
            reply->writeInt32(ret);
            break;
        }
        case CMD_AS_DSMV: {
            int ret = Device_set_master_volume(data.readFloat());
            reply->writeInt32(ret);
            break;
        }
        case CMD_AS_DGMV: {
            float masterVolume;
            int ret = Device_get_master_volume(masterVolume);
            reply->writeInt32(ret);
            reply->writeFloat(masterVolume);
            break;
        }
        case CMD_AS_DSM: {
            audio_mode_t mode = (audio_mode_t) data.readInt32();
            int ret = Device_set_mode(mode);
            reply->writeInt32(ret);
            break;
        }
        case CMD_AS_DSMicM: {
            bool state = data.readInt32();
            int ret = Device_set_mic_mute(state);
            reply->writeInt32(ret);
            break;
        }
        case CMD_AS_DGMicM: {
            bool state;
            int ret = Device_get_mic_mute(state);
            reply->writeInt32(ret);

            int stateInt = state;
            reply->writeInt32(stateInt);
            break;
        }
        case CMD_AS_DSP: {
            const char* kv_pairs = data.readCString();
            int ret = Device_set_parameters(kv_pairs);
            reply->writeInt32(ret);
            break;
        }
        case CMD_AS_DGP: {
            const char* keys = data.readCString();
            char* params = Device_get_parameters(keys);
            reply->writeCString(params);
            break;
        }
        case CMD_AS_DGIBS: {
            struct audio_config config; readAudioConfigFromParcel(config, data);
            int ret = Device_get_input_buffer_size(config);
            reply->writeInt32(ret);
            break;
        }
        case CMD_AS_DOOS: {
            ::android::sp<::android::IBinder> audioClientBinder = data.readStrongBinder();
            int seq = data.readInt32();
            audio_io_handle_t handle = (audio_io_handle_t) data.readInt32();
            audio_devices_t devices = (audio_devices_t) data.readUint32();
            audio_output_flags_t flags = (audio_output_flags_t) data.readInt32();
            struct audio_config config; readAudioConfigFromParcel(config, data);
            const char* address = data.readCString();

            int clientId;
            int ret = Device_open_output_stream(audioClientBinder, seq, handle, devices, flags, config, address, clientId);

            reply->writeInt32(clientId);
            reply->writeInt32(ret);
            break;
        }
        case CMD_AS_DCOS: {
            const char* name = data.readCString();
            Device_close_output_stream(name);
            break;
        }
        case CMD_AS_DOIS: {
            ::android::sp<::android::IBinder> audioClientBinder = data.readStrongBinder();
            int seq = data.readInt32();
            audio_io_handle_t handle = (audio_io_handle_t) data.readInt32();
            audio_devices_t devices = (audio_devices_t) data.readUint32();
            struct audio_config config; readAudioConfigFromParcel(config, data);
            audio_input_flags_t flags = (audio_input_flags_t) data.readInt32();
            const char* address = data.readCString();
            audio_source_t source = (audio_source_t) data.readInt32();

            int clientId;
            int ret = Device_open_input_stream(audioClientBinder, seq, handle, devices, config, flags, address, source, clientId);

            reply->writeInt32(clientId);
            reply->writeInt32(ret);
            break;
        }
        case CMD_AS_DCIS: {
            const char* name = data.readCString();
            Device_close_input_stream(name);
            break;
        }
        case CMD_AS_DD: {
            std::string deviceDump;
            int ret = Device_dump(deviceDump);
            reply->writeInt32(ret);
            reply->writeCString(deviceDump.c_str());
            break;
        }
        case CMD_AS_DSMasterM: {
            bool mute = data.readInt32();
            int ret = Device_set_master_mute(mute);
            reply->writeInt32(ret);
            break;
        }
        case CMD_AS_DGMasterM: {
            bool mute;
            int ret = Device_get_master_mute(mute);
            reply->writeInt32(ret);

            int muteInt = mute;
            reply->writeInt32(muteInt);
            break;
        }
        case CMD_AS_DCAP: {
            unsigned int dcap_num_sources, dcap_num_sinks;
            dcap_num_sources = data.readUint32(), dcap_num_sinks = data.readUint32();
            struct audio_port_config* dcapSources = new audio_port_config[dcap_num_sources], *dcapSinks = new audio_port_config[dcap_num_sinks];
            readAudioPortConfigsFromParcel(dcap_num_sources, dcapSources, data);
            readAudioPortConfigsFromParcel(dcap_num_sinks, dcapSinks, data);

            audio_patch_handle_t handle;
            int ret = Device_create_audio_patch(dcap_num_sources, dcapSources, dcap_num_sinks, dcapSinks, handle);
            delete [] dcapSources; delete [] dcapSinks;
            reply->writeInt32(ret);
            reply->writeInt32(handle);
            break;
        }
        case CMD_AS_DRAP: {
            audio_patch_handle_t handle;
            data.read(&handle, sizeof(audio_patch_handle_t));
            int ret = Device_release_audio_patch(handle);
            reply->writeInt32(ret);
            break;
        }
        case CMD_AS_DSAPC: {
            struct audio_port_config config; readAudioPortConfigFromParcel(config, data);
            int ret = Device_set_audio_port_config(config);
            reply->writeInt32(ret);
            break;
        }
        case CMD_AS_SGSR: {
            const char* name = data.readCString();
            uint32_t ret = Stream_get_sample_rate(name);
            reply->writeUint32(ret);
            break;
        }
        case CMD_AS_SGBS: {
            const char* name = data.readCString();
            int ret = Stream_get_buffer_size(name);
            reply->writeInt32(ret);
            break;
        }
        case CMD_AS_SGC: {
            const char* name = data.readCString();
            uint32_t ret = Stream_get_channels(name);

            // ret is channel, not return code
            reply->writeUint32(ret);
            break;
        }
        case CMD_AS_SGF: {
            const char* name = data.readCString();
            uint32_t ret = Stream_get_format(name);

            // ret is format, not return code
            reply->writeUint32(ret);
            break;
        }
        case CMD_AS_SS: {
            const char* name = data.readCString();
            uint32_t ret = Stream_standby(name);
            reply->writeUint32(ret);
            break;
        }
        case CMD_AS_SGD: {
            const char* name = data.readCString();
            uint32_t ret = Stream_get_device(name);
            reply->writeUint32(ret);
            break;
        }
        case CMD_AS_SSP: {
            const char* name = data.readCString(),
                    *kv_pairs = data.readCString();
            int ret = Stream_set_parameters(name, kv_pairs);
            reply->writeInt32(ret);
            break;
        }
        case CMD_AS_SGP: {
            const char* name = data.readCString(),
                    *keys = data.readCString();
            char* params = Stream_get_parameters(name, keys);
            reply->writeCString(params);
            break;
        }
        case CMD_AS_SO_GL: {
            const char* name = data.readCString();
            uint32_t ret = StreamOut_get_latency(name);
            reply->writeUint32(ret);
            break;
        }
        case CMD_AS_SO_SV: {
            const char* name = data.readCString();
            float left = data.readFloat();
            float right = data.readFloat();
            int ret = StreamOut_set_volume(name, left, right);
            reply->writeInt32(ret);
            break;
        }
        case CMD_AS_SO_W: {
            const char* name = data.readCString();
            size_t bytes; data.read(&bytes, sizeof(bytes));
            int ret = StreamOut_write(name, bytes);
            reply->writeInt32(ret);
            break;
        }
        case CMD_AS_SO_GRP: {
            const char* name = data.readCString();
            uint32_t dsp_frames;
            int ret = StreamOut_get_render_position(name, dsp_frames);
            reply->writeInt32(ret);
            reply->writeUint32(dsp_frames);
            break;
        }
        case CMD_AS_SO_GNWT: {
            const char* name = data.readCString();
            int64_t timestamp;
            int ret = StreamOut_get_next_write_timestamp(name, timestamp);
            reply->writeInt32(ret);
            reply->writeInt64(timestamp);
            break;
        }
        case CMD_AS_SO_P: {
            const char* name = data.readCString();
            int ret = StreamOut_pause(name);
            reply->writeInt32(ret);
            break;
        }
        case CMD_AS_SO_R: {
            const char* name = data.readCString();
            int ret = StreamOut_resume(name);
            reply->writeInt32(ret);
            break;
        }
        case CMD_AS_SO_F: {
            const char* name = data.readCString();
            int ret = StreamOut_flush(name);
            reply->writeInt32(ret);
            break;
        }
        case CMD_AS_SO_GPP: {
            const char* name = data.readCString();
            uint64_t frames; struct timespec timestamp;
            int ret = StreamOut_get_presentation_position(name, frames, timestamp);
            reply->writeInt32(ret);
            reply->writeUint64(frames);
            reply->writeInt64((int64_t) timestamp.tv_sec);
            reply->write(&timestamp.tv_nsec, sizeof(timestamp.tv_nsec));
            break;
        }
        case CMD_AS_SI_SG: {
            const char* name = data.readCString();
            float gain = data.readFloat();
            int ret = StreamIn_set_gain(name, gain);
            reply->writeInt32(ret);
            break;
        }
        case CMD_AS_SI_R: {
            const char* name = data.readCString();
            size_t bytes; data.read(&bytes, sizeof(bytes));
            int ret = StreamIn_read(name, bytes);
            reply->writeInt32(ret);
            break;
        }
        case CMD_AS_SI_GIFL: {
            const char* name = data.readCString();
            uint32_t ret = StreamIn_get_input_frames_lost(name);
            reply->writeUint32(ret);
            break;
        }
        case CMD_AS_SI_GCP: {
            const char* name = data.readCString();
            int64_t frames, time;
            int ret = StreamIn_get_capture_position(name, frames, time);
            reply->writeInt32(ret);
            reply->writeInt64(frames);
            reply->writeInt64(time);
            break;
        }
        case CMD_AS_SP: {
            int status_32 = -999;
            int ret = Service_ping(status_32);
            reply->writeInt32(ret);
            reply->writeCString(std::to_string(status_32).c_str());
            break;
        }
        case CMD_AS_ESP: {
            aml_audio_effect_type_e type = (aml_audio_effect_type_e) data.readInt32();
            uint32_t cmdSize = data.readUint32();
            const char* pCmdDataCStr = data.readCString();
            void* pCmdData = (void *) pCmdDataCStr;
            uint32_t replySize = data.readUint32();

            uint32_t pReplyData;
            int ret = Effect_set_parameters(type, cmdSize, pCmdData, replySize, pReplyData);
            reply->writeUint32(pReplyData);
            reply->writeInt32(ret);
            break;
        }
        case CMD_AS_EGP: {
            aml_audio_effect_type_e type = (aml_audio_effect_type_e) data.readInt32();
            uint32_t cmdSize = data.readUint32();
            const char* pCmdDataCStr = data.readCString();
            void* pCmdData = (void *) pCmdDataCStr;
            uint32_t replySize = data.readUint32();

            void* pReplyData = malloc(replySize);
            int ret = Effect_get_parameters(type, cmdSize, pCmdData, replySize, pReplyData);
            reply->writeInt32(ret);

            if (!ret) {
                reply->writeUint32(replySize);
                reply->write(pReplyData, replySize);
            }

            free(pReplyData);
            pReplyData = nullptr;
            break;
        }
        default:
            return ::android::BBinder::onTransact(code, data, reply, flags);
    }

    return (0);
}