/*
 * libmerc_test.c
 *
 * main() file for libmerc.so test driver program
 *
 * Copyright (c) 2020 Cisco Systems, Inc. All rights reserved.  License at
 * https://github.com/cisco/mercury/blob/master/LICENSE
 */

#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <strings.h>
#include <pthread.h>
#include <array>

#include "libmerc/libmerc.h"

namespace snort {
#define SO_PUBLIC

    SO_PUBLIC void LogMessage(const char*, ...) __attribute__((format (printf, 1, 2)));
    SO_PUBLIC void LogMessage(FILE*, const char*, ...) __attribute__((format (printf, 2, 3)));
    SO_PUBLIC void WarningMessage(const char*, ...) __attribute__((format (printf, 1, 2)));
    SO_PUBLIC void ErrorMessage(const char*, ...) __attribute__((format (printf, 1, 2)));

    [[noreturn]] SO_PUBLIC void FatalError(const char*, ...) __attribute__((format (printf, 1, 2)));
}

unsigned char client_hello_eth[] = {
  0x00, 0x50, 0x56, 0xe0, 0xb0, 0xbc, 0x00, 0x0c, 0x29, 0x74, 0x82, 0x2f,
  0x08, 0x00, 0x45, 0x00, 0x01, 0x61, 0xd5, 0xeb, 0x40, 0x00, 0x40, 0x06,
  0x58, 0x0c, 0xc0, 0xa8, 0x71, 0xed, 0x97, 0x65, 0x41, 0xa4, 0x80, 0x2a,
  0x01, 0xbb, 0xdd, 0x07, 0xfe, 0x40, 0x25, 0x00, 0x2e, 0x63, 0x50, 0x18,
  0xfa, 0xf0, 0x0c, 0xf3, 0x00, 0x00, 0x16, 0x03, 0x01, 0x01, 0x34, 0x01,
  0x00, 0x01, 0x30, 0x03, 0x03, 0x5b, 0x1f, 0x43, 0x3b, 0x2f, 0x09, 0x1c,
  0x61, 0xff, 0xd5, 0x1d, 0x3d, 0x8f, 0x00, 0x8f, 0xea, 0x86, 0x3f, 0xb6,
  0xc3, 0x72, 0x6e, 0x7f, 0x05, 0x6b, 0x01, 0x9e, 0xc7, 0x68, 0xcd, 0x12,
  0x58, 0x20, 0xf0, 0xa3, 0x04, 0x3a, 0x4f, 0x60, 0x89, 0x7b, 0x16, 0x89,
  0xf7, 0x46, 0xcf, 0x3c, 0x69, 0x03, 0xf9, 0xf6, 0x06, 0xa7, 0x7f, 0x53,
  0x36, 0xd4, 0xe2, 0x16, 0x33, 0xe9, 0x88, 0x48, 0xff, 0x14, 0x00, 0x3e,
  0x13, 0x02, 0x13, 0x03, 0x13, 0x01, 0xc0, 0x2c, 0xc0, 0x30, 0x00, 0x9f,
  0xcc, 0xa9, 0xcc, 0xa8, 0xcc, 0xaa, 0xc0, 0x2b, 0xc0, 0x2f, 0x00, 0x9e,
  0xc0, 0x24, 0xc0, 0x28, 0x00, 0x6b, 0xc0, 0x23, 0xc0, 0x27, 0x00, 0x67,
  0xc0, 0x0a, 0xc0, 0x14, 0x00, 0x39, 0xc0, 0x09, 0xc0, 0x13, 0x00, 0x33,
  0x00, 0x9d, 0x00, 0x9c, 0x00, 0x3d, 0x00, 0x3c, 0x00, 0x35, 0x00, 0x2f,
  0x00, 0xff, 0x01, 0x00, 0x00, 0xa9, 0x00, 0x00, 0x00, 0x10, 0x00, 0x0e,
  0x00, 0x00, 0x0b, 0x6e, 0x79, 0x74, 0x69, 0x6d, 0x65, 0x73, 0x2e, 0x63,
  0x6f, 0x6d, 0x00, 0x0b, 0x00, 0x04, 0x03, 0x00, 0x01, 0x02, 0x00, 0x0a,
  0x00, 0x0c, 0x00, 0x0a, 0x00, 0x1d, 0x00, 0x17, 0x00, 0x1e, 0x00, 0x19,
  0x00, 0x18, 0x00, 0x23, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x17,
  0x00, 0x00, 0x00, 0x0d, 0x00, 0x30, 0x00, 0x2e, 0x04, 0x03, 0x05, 0x03,
  0x06, 0x03, 0x08, 0x07, 0x08, 0x08, 0x08, 0x09, 0x08, 0x0a, 0x08, 0x0b,
  0x08, 0x04, 0x08, 0x05, 0x08, 0x06, 0x04, 0x01, 0x05, 0x01, 0x06, 0x01,
  0x03, 0x03, 0x02, 0x03, 0x03, 0x01, 0x02, 0x01, 0x03, 0x02, 0x02, 0x02,
  0x04, 0x02, 0x05, 0x02, 0x06, 0x02, 0x00, 0x2b, 0x00, 0x09, 0x08, 0x03,
  0x04, 0x03, 0x03, 0x03, 0x02, 0x03, 0x01, 0x00, 0x2d, 0x00, 0x02, 0x01,
  0x01, 0x00, 0x33, 0x00, 0x26, 0x00, 0x24, 0x00, 0x1d, 0x00, 0x20, 0x7f,
  0x36, 0x7d, 0x60, 0x25, 0x06, 0x55, 0xca, 0xbb, 0x18, 0xd3, 0x4c, 0x84,
  0xcc, 0x5b, 0x14, 0xcd, 0x0a, 0x95, 0xe9, 0x06, 0x13, 0x5d, 0xd7, 0x6a,
  0xee, 0x62, 0x2b, 0x2b, 0x54, 0x1c, 0x17
};
size_t client_hello_eth_len = sizeof(client_hello_eth);

unsigned char tcp_syn[] = {
  0x00, 0x50, 0x56, 0x8e, 0x1d, 0xdc, 0x00, 0x50, 0x56, 0xa2, 0x71, 0xbf,
  0x08, 0x00, 0x45, 0x00, 0x00, 0x34, 0x58, 0x5b, 0x40, 0x00, 0x80, 0x06,
  0x41, 0x99, 0x0a, 0x0a, 0x01, 0x70, 0x0d, 0xf9, 0x47, 0x5d, 0xd2, 0x7e,
  0x01, 0xbb, 0x68, 0x4d, 0xc1, 0x2d, 0x00, 0x00, 0x00, 0x00, 0x80, 0x02,
  0xfa, 0xf0, 0x15, 0x9b, 0x00, 0x00, 0x02, 0x04, 0x05, 0xb4, 0x01, 0x03,
  0x03, 0x08, 0x01, 0x01, 0x04, 0x02
};

unsigned char firefox_client_hello_eth[] = {
  0x00, 0x50, 0x56, 0x8e, 0x1d, 0xdc, 0x00, 0x50, 0x56, 0xa2, 0x71, 0xbf,
  0x08, 0x00, 0x45, 0x00, 0x02, 0x2d, 0x58, 0x5d, 0x40, 0x00, 0x80, 0x06,
  0x3f, 0x9e, 0x0a, 0x0a, 0x01, 0x70, 0x0d, 0xf9, 0x47, 0x5d, 0xd2, 0x7e,
  0x01, 0xbb, 0x68, 0x4d, 0xc1, 0x2e, 0xb1, 0x1b, 0xb4, 0x8d, 0x50, 0x18,
  0x04, 0x00, 0xd6, 0x50, 0x00, 0x00, 0x16, 0x03, 0x01, 0x02, 0x00, 0x01,
  0x00, 0x01, 0xfc, 0x03, 0x03, 0x3e, 0xe1, 0x7f, 0xbd, 0x7c, 0xbc, 0x30,
  0xdb, 0xa4, 0xf4, 0x74, 0xbe, 0x4a, 0x08, 0xa4, 0x8b, 0xf1, 0x5a, 0x82,
  0x78, 0x8a, 0x91, 0xf7, 0x00, 0xec, 0x25, 0xed, 0x6e, 0x5f, 0xea, 0xfb,
  0x3c, 0x20, 0x98, 0x79, 0xf8, 0xb5, 0x31, 0xed, 0x1d, 0xf8, 0x9c, 0x90,
  0x04, 0xc2, 0x0c, 0x14, 0xb8, 0x43, 0x34, 0x8e, 0x10, 0x88, 0x12, 0xa3,
  0x9a, 0x9a, 0x86, 0xab, 0x98, 0x06, 0x65, 0x4f, 0x4a, 0x86, 0x00, 0x24,
  0x13, 0x01, 0x13, 0x03, 0x13, 0x02, 0xc0, 0x2b, 0xc0, 0x2f, 0xcc, 0xa9,
  0xcc, 0xa8, 0xc0, 0x2c, 0xc0, 0x30, 0xc0, 0x0a, 0xc0, 0x09, 0xc0, 0x13,
  0xc0, 0x14, 0x00, 0x9c, 0x00, 0x9d, 0x00, 0x2f, 0x00, 0x35, 0x00, 0x0a,
  0x01, 0x00, 0x01, 0x8f, 0x00, 0x00, 0x00, 0x2a, 0x00, 0x28, 0x00, 0x00,
  0x25, 0x66, 0x69, 0x72, 0x65, 0x66, 0x6f, 0x78, 0x2e, 0x73, 0x65, 0x74,
  0x74, 0x69, 0x6e, 0x67, 0x73, 0x2e, 0x73, 0x65, 0x72, 0x76, 0x69, 0x63,
  0x65, 0x73, 0x2e, 0x6d, 0x6f, 0x7a, 0x69, 0x6c, 0x6c, 0x61, 0x2e, 0x63,
  0x6f, 0x6d, 0x00, 0x17, 0x00, 0x00, 0xff, 0x01, 0x00, 0x01, 0x00, 0x00,
  0x0a, 0x00, 0x0e, 0x00, 0x0c, 0x00, 0x1d, 0x00, 0x17, 0x00, 0x18, 0x00,
  0x19, 0x01, 0x00, 0x01, 0x01, 0x00, 0x0b, 0x00, 0x02, 0x01, 0x00, 0x00,
  0x23, 0x00, 0x00, 0x00, 0x10, 0x00, 0x0b, 0x00, 0x09, 0x08, 0x68, 0x74,
  0x74, 0x70, 0x2f, 0x31, 0x2e, 0x31, 0x00, 0x05, 0x00, 0x05, 0x01, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x33, 0x00, 0x6b, 0x00, 0x69, 0x00, 0x1d, 0x00,
  0x20, 0xc0, 0xab, 0xaa, 0x1f, 0xd2, 0x71, 0xdc, 0x6f, 0xc8, 0x7b, 0x4b,
  0x32, 0x65, 0xff, 0x51, 0x6c, 0x41, 0x07, 0xff, 0x9a, 0x9c, 0x54, 0x30,
  0x90, 0xbc, 0x42, 0xcf, 0x5d, 0xda, 0x3d, 0xd6, 0x57, 0x00, 0x17, 0x00,
  0x41, 0x04, 0x04, 0xbc, 0x09, 0x6a, 0x69, 0xf7, 0xc5, 0xda, 0xe6, 0x01,
  0x17, 0x9c, 0x43, 0x3d, 0x59, 0xa2, 0x78, 0xfc, 0x68, 0xeb, 0x31, 0x63,
  0x7a, 0x1c, 0xba, 0xdf, 0xe6, 0x3b, 0x9a, 0x88, 0x08, 0x36, 0x06, 0xcb,
  0xf5, 0xff, 0x08, 0x76, 0xcd, 0xb1, 0xb2, 0xdc, 0x19, 0x88, 0x1a, 0x9a,
  0x77, 0xaa, 0x4f, 0x93, 0x80, 0xdf, 0xf3, 0x7d, 0x03, 0x35, 0x63, 0x24,
  0x7c, 0x88, 0x72, 0x5c, 0xef, 0x30, 0x00, 0x2b, 0x00, 0x05, 0x04, 0x03,
  0x04, 0x03, 0x03, 0x00, 0x0d, 0x00, 0x18, 0x00, 0x16, 0x04, 0x03, 0x05,
  0x03, 0x06, 0x03, 0x08, 0x04, 0x08, 0x05, 0x08, 0x06, 0x04, 0x01, 0x05,
  0x01, 0x06, 0x01, 0x02, 0x03, 0x02, 0x01, 0x00, 0x2d, 0x00, 0x02, 0x01,
  0x01, 0x00, 0x1c, 0x00, 0x02, 0x40, 0x01, 0x00, 0x15, 0x00, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

unsigned char client_hello_no_server_name_eth[] = {
  0x00, 0x26, 0x98, 0x0b, 0x01, 0x42, 0x3c, 0x08, 0xf6, 0xd8, 0xf0, 0xc7,
  0x08, 0x00, 0x45, 0x00, 0x01, 0x11, 0x4a, 0x96, 0x40, 0x00, 0x7c, 0x06,
  0x3a, 0xa4, 0x0a, 0x52, 0xd6, 0x87, 0x2c, 0xe0, 0x6a, 0xf3, 0xd6, 0x93,
  0x01, 0xbb, 0x60, 0xac, 0xd1, 0x8b, 0x5f, 0x9e, 0xb5, 0x50, 0x50, 0x18,
  0x01, 0x04, 0x1c, 0x92, 0x00, 0x00, 0x16, 0x03, 0x01, 0x00, 0xe4, 0x01,
  0x00, 0x00, 0xe0, 0x03, 0x03, 0x2a, 0x09, 0x12, 0xec, 0x90, 0x63, 0xdf,
  0x49, 0x4b, 0xe4, 0x01, 0xbf, 0x5e, 0xbb, 0x87, 0xa1, 0x12, 0x3f, 0xa0,
  0x44, 0x63, 0x74, 0x28, 0xc1, 0x85, 0xc8, 0xc3, 0x1a, 0xab, 0xa4, 0x41,
  0x4a, 0x20, 0xe8, 0x13, 0x57, 0xf1, 0xf9, 0xfd, 0x5d, 0x83, 0x5d, 0xb8,
  0x10, 0xe1, 0x2a, 0x01, 0xca, 0x92, 0x72, 0xb7, 0x8d, 0x0b, 0xe6, 0x87,
  0x90, 0x76, 0x82, 0x77, 0x56, 0xf0, 0x9a, 0x27, 0xb7, 0xe3, 0x00, 0x42,
  0xc0, 0x30, 0xc0, 0x2c, 0xc0, 0x28, 0xc0, 0x24, 0x00, 0xa5, 0x00, 0xa1,
  0x00, 0x9f, 0x00, 0x6b, 0x00, 0x69, 0x00, 0x68, 0xc0, 0x32, 0xc0, 0x2e,
  0xc0, 0x2a, 0xc0, 0x26, 0x00, 0x9d, 0x00, 0x3d, 0xc0, 0x2f, 0xc0, 0x2b,
  0xc0, 0x27, 0xc0, 0x23, 0x00, 0xa4, 0x00, 0xa0, 0x00, 0x9e, 0x00, 0x67,
  0x00, 0x3f, 0x00, 0x3e, 0xc0, 0x31, 0xc0, 0x2d, 0xc0, 0x29, 0xc0, 0x25,
  0x00, 0x9c, 0x00, 0x3c, 0x00, 0xff, 0x01, 0x00, 0x00, 0x55, 0x00, 0x0b,
  0x00, 0x04, 0x03, 0x00, 0x01, 0x02, 0x00, 0x0a, 0x00, 0x1c, 0x00, 0x1a,
  0x00, 0x17, 0x00, 0x19, 0x00, 0x1c, 0x00, 0x1b, 0x00, 0x18, 0x00, 0x1a,
  0x00, 0x16, 0x00, 0x0e, 0x00, 0x0d, 0x00, 0x0b, 0x00, 0x0c, 0x00, 0x09,
  0x00, 0x0a, 0x00, 0x23, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x20, 0x00, 0x1e,
  0x06, 0x01, 0x06, 0x02, 0x06, 0x03, 0x05, 0x01, 0x05, 0x02, 0x05, 0x03,
  0x04, 0x01, 0x04, 0x02, 0x04, 0x03, 0x03, 0x01, 0x03, 0x02, 0x03, 0x03,
  0x02, 0x01, 0x02, 0x02, 0x02, 0x03, 0x00, 0x0f, 0x00, 0x01, 0x01
};

// end of packet data

typedef void (*dummy_func)();

struct libmerc_api {
    mercury_context (*mercury_init)(const libmerc_config& vars, int verbosity);
    int (*mercury_finalize)(mercury_context);
    size_t (*mercury_analyze)(mercury_packet_processor processor, void* buffer,
        size_t buffer_size, const uint8_t* packet, size_t length, struct timespec* ts);
    mercury_packet_processor (*mercury_packet_processor_construct)(mercury_context mc);
    void (*mercury_packet_processor_destruct)(mercury_packet_processor mpp);
    const analysis_context* (*get_analysis_context)(mercury_packet_processor processor,
        const uint8_t* packet, size_t length, struct timespec* ts);
    enum fingerprint_type (*get_fingerprint_type)(const struct analysis_context* ac);
    enum fingerprint_status (*get_fingerprint_status)(const analysis_context* ac);
    bool (*get_process_info)(const analysis_context* ac, const char** probable_process,
        double* probability_score);
    bool (*get_malware_info)(const analysis_context* ac, bool* probable_process_is_malware,
        double* probability_malware);

    bool (*write_stats_data)(mercury_context mc, const char *stats_data_file_path);

    void* mercury_handle = nullptr;
};

libmerc_api mercury{};

int mercury_bind(struct libmerc_api &mercury_api, const char *lib_path) {

    if ((mercury_api.mercury_handle = dlopen(lib_path, RTLD_LAZY|RTLD_LOCAL)) == nullptr) {
        const char *dlerr = dlerror();
        fprintf(stderr, "mercury: failed to load %s: %s\n", lib_path, dlerr ? dlerr : "unknown error");
        return -1; // error
    } else {
        fprintf(stderr, "mercury: loading %s\n", lib_path);
    }

    struct FuncBinding
    {
        const char* lib_sym;
        dummy_func* local_sym;
    } bindings[] =
    {
        { "mercury_init", (dummy_func*)&mercury_api.mercury_init },
        { "mercury_finalize", (dummy_func*)&mercury_api.mercury_finalize },
        { "mercury_packet_processor_write_json", (dummy_func*)&mercury_api.mercury_analyze },
        { "mercury_packet_processor_construct", (dummy_func*)&mercury_api.mercury_packet_processor_construct },
        { "mercury_packet_processor_destruct", (dummy_func*)&mercury_api.mercury_packet_processor_destruct },
        { "mercury_packet_processor_get_analysis_context", (dummy_func*)&mercury_api.get_analysis_context },
        { "analysis_context_get_fingerprint_type", (dummy_func*)&mercury_api.get_fingerprint_type },
        { "analysis_context_get_fingerprint_status", (dummy_func*)&mercury_api.get_fingerprint_status },
        { "analysis_context_get_process_info", (dummy_func*)&mercury_api.get_process_info },
        { "analysis_context_get_malware_info", (dummy_func*)&mercury_api.get_malware_info },
        { "mercury_write_stats_data", (dummy_func*)&mercury_api.write_stats_data },
        { nullptr, nullptr }
    };

    FuncBinding* index;
    dlerror();  // initialize error reporting
    for (index = bindings; index->lib_sym; index++) {
        *(void**)index->local_sym  = dlsym(mercury_api.mercury_handle, index->lib_sym);
        if (*(index->local_sym) == nullptr) {
            const char *dlerr = dlerror();
            fprintf(stderr, "mercury: failed to resolve symbol: %s (error: %s)\n", index->lib_sym, dlerr ? dlerr : "unknown error");
            return -2; // error
        }
    }

    fprintf(stderr, "mercury_bind() succeeded with handle %p\n", mercury_api.mercury_handle);

    return 0; // success
}

void mercury_unbind(struct libmerc_api &libmerc_api) {
    dlclose(libmerc_api.mercury_handle);
    libmerc_api.mercury_handle = nullptr;
}

struct packet_processor_state {
    unsigned int thread_number;
    struct libmerc_api *mercury;
    mercury_context mc;

    packet_processor_state(unsigned int tn, struct libmerc_api *m, mercury_context c) : thread_number{tn}, mercury{m}, mc{c} {}

};

void *packet_processor(void *arg) {
    packet_processor_state *pp = (packet_processor_state *)arg;
    struct libmerc_api *merc = pp->mercury;
    struct timespec time;
    time.tv_sec = time.tv_nsec = 0;  // set to January 1st, 1970 (the Epoch)

    fprintf(stderr, "packet_processor() has libmerc_api=%p and mercury_context=%p\n", (void *)merc, (void *)pp->mc);

    // create mercury packet processor
    mercury_packet_processor mpp = merc->mercury_packet_processor_construct(pp->mc);
    if (mpp == NULL) {
        fprintf(stderr, "error in mercury_packet_processor_construct()\n");
        return NULL;
    }

    // get analysis results and print process name and score
    //
    const char *probable_process = NULL;
    double probability_score = 0.0;
    const struct analysis_context *ctx = merc->get_analysis_context(mpp, client_hello_eth, client_hello_eth_len, &time);
    if (ctx == NULL) {
        fprintf(stderr, "error in get_analysis_context()\n");
        return NULL;
    }
    if (merc->get_process_info(ctx, &probable_process, &probability_score)) {
        fprintf(stderr, "thread: %u\tprobable_process: %s\tscore: %f\n", pp->thread_number, probable_process, probability_score);
    }

    // try it on another packet
    //
    probable_process = NULL;
    probability_score = 0.0;
    ctx = merc->get_analysis_context(mpp, client_hello_no_server_name_eth, sizeof(client_hello_no_server_name_eth), &time);
    if (ctx == NULL) {
        fprintf(stderr, "error in get_analysis_context()\n");
        return NULL;
    }
    if (merc->get_process_info(ctx, &probable_process, &probability_score)) {
        fprintf(stderr, "thread: %u\tprobable_process: %s\tscore: %f\n", pp->thread_number, probable_process, probability_score);
    }

    // destroy packet processor
    merc->mercury_packet_processor_destruct(mpp);

    return NULL;
}

int test_libmerc(struct libmerc_config *config, int verbosity, bool fail=false) {
    int num_loops = 4;
    constexpr int num_threads = 8;

    for (int i = 0; i < num_loops; i++) {
        fprintf(stderr, "loop: %d\n", i);

        // bind libmerc
        int retval = mercury_bind(mercury, "./libmerc/libmerc.so");
        if (retval != 0) {
            fprintf(stderr, "error: mercury_bind() returned code %d\n", retval);
            return 1;
        }

        // init mercury
        mercury_context mc = mercury.mercury_init(*config, verbosity);
        if (mc == NULL) {
            fprintf(stderr, "error: mercury_init() returned null\n");
            return -1;
        }

        // create packet processing threads
        std::array<pthread_t, num_threads> tid_array;
        packet_processor_state thread_state[num_threads] = {
             { 0, &mercury, mc },
             { 1, &mercury, mc },
             { 2, &mercury, mc },
             { 3, &mercury, mc },
             { 4, &mercury, mc },
             { 5, &mercury, mc },
             { 6, &mercury, mc },
             { 7, &mercury, mc }
            };
        //std::array<unsigned int, num_threads> thread_number = { 0, 1, 2, 3, 4, 5, 6, 7 };
        for (int idx=0; idx < num_threads; idx++) {
            pthread_create(&tid_array[idx], NULL, packet_processor, &thread_state[idx]);
        }
        fprintf(stderr, "created all %zu threads\n", tid_array.size());

        if (fail) {
            // delete mercury state, to force failure
            mercury.mercury_finalize(mc);
        }

        for (auto & t : tid_array) {
            pthread_join(t, NULL);
        }
        fprintf(stderr, "joined all %zu threads\n", tid_array.size());

        // write stats file
        mercury.write_stats_data(mc, "libmerc_driver_stats.json.gz");

        // destroy mercury
        mercury.mercury_finalize(mc);

        fprintf(stderr, "completed mercury_finalize()\n");

        // unbind libmerc
        mercury_unbind(mercury);

    }

    return 0;
}

int double_bind_test(struct libmerc_config *config, struct libmerc_config *config2) {
    int verbosity = 1;
    int num_loops = 4;
    constexpr int num_threads = 8;

    fprintf(stderr, "running mercury_double_bind() test\n");

    for (int i = 0; i < num_loops; i++) {
        fprintf(stderr, "loop: %d\n", i);

        // bind libmerc
        int retval = mercury_bind(mercury, "./libmerc/libmerc.so");
        if (retval != 0) {
            fprintf(stderr, "error: mercury_bind() returned code %d\n", retval);
            return 1;
        }

        // init mercury
        mercury_context mc = mercury.mercury_init(*config, verbosity);
        if (mc == nullptr) {
            fprintf(stderr, "error: mercury_init() returned null\n");
            return -1;
        }

        // bind and init second mercury library
        struct libmerc_api mercury_alt;
        retval = mercury_bind(mercury_alt, "./libmerc/libmerc.so.alt");
        if (retval != 0) {
            fprintf(stderr, "error: mercury_bind() returned code %d in second bind\n", retval);
            return 1;
        }
        mercury_context mc_alt = mercury_alt.mercury_init(*config2, verbosity);
        if (mc_alt == nullptr) {
            fprintf(stderr, "error: mercury_init() returned null in second init\n");
            return -1;
        }

        // create packet processing threads
        std::array<pthread_t, num_threads> tid_array;
        packet_processor_state thread_state[num_threads] = {
             { 0, &mercury, mc },
             { 1, &mercury, mc },
             { 2, &mercury, mc },
             { 3, &mercury, mc },
             { 4, &mercury_alt, mc_alt },
             { 5, &mercury_alt, mc_alt },
             { 6, &mercury_alt, mc_alt },
             { 7, &mercury_alt, mc_alt }
            };
        for (int idx=0; idx < num_threads; idx++) {
            pthread_create(&tid_array[idx], NULL, packet_processor, &thread_state[idx]);
        }
        fprintf(stderr, "created all %zu threads\n", tid_array.size());

        mercury.write_stats_data(mc, "libmerc_driver_stats_pre_join.json.gz");
        mercury.write_stats_data(mc_alt, "libmerc_driver_stats_pre_join_alt.json.gz");

        for (auto & t : tid_array) {
            pthread_join(t, NULL);
        }
        fprintf(stderr, "joined all %zu threads\n", tid_array.size());

        // write stats file
        mercury.write_stats_data(mc, "libmerc_driver_stats_post_join.json.gz");
        mercury.write_stats_data(mc_alt, "libmerc_driver_stats_post_join_alt.json.gz");

        // destroy mercury
        mercury.mercury_finalize(mc);

        fprintf(stderr, "completed mercury_finalize()\n");

        // unbind libmerc
        mercury_unbind(mercury);

        mercury_alt.mercury_finalize(mc_alt);
        mercury_unbind(mercury_alt);

    }

    return 0;
}


int main(int , char *[]) {
    int verbosity = 1;

    // initialize libmerc's global configuration by creating a
    // libmerc_config structure and then passing it into mercury_init
    //
    libmerc_config config{};
    config.do_analysis = true;
    config.do_stats = true;
    std::string resources_path = "../resources/resources.tgz";
    config.resources = (char*) resources_path.c_str();

    libmerc_config config_lite{};
    config_lite.do_analysis = true;
    config_lite.do_stats = true;
    std::string resources_lite_path = "../resources/resources_lite.tgz";
    config_lite.resources = (char*) resources_lite_path.c_str();

    bool perform_double_bind_test = true;
    if (perform_double_bind_test) {

        // perform double bind/init test
        int retval = double_bind_test(&config_lite, &config);
        if (retval) {
            fprintf(stderr, "double_bind_test() error (code %d)\n", retval);
            return EXIT_FAILURE;
        }
    }

    int retval = test_libmerc(&config, verbosity);
    if (retval) {
        fprintf(stderr, "test_libmerc() error (code %d)\n", retval);
        return EXIT_FAILURE;
    }

    retval = test_libmerc(&config_lite, verbosity);
    if (retval) {
        fprintf(stderr, "test_libmerc() error (code %d)\n", retval);
        return EXIT_FAILURE;
    }

    // repeat test with original config
    //
    retval = test_libmerc(&config, verbosity);
    if (retval) {
        fprintf(stderr, "test_libmerc() error (code %d)\n", retval);
        return EXIT_FAILURE;
    }

    return 0;
}
