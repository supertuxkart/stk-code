#ifdef ENABLE_RECORDER
#ifndef HEADER_RECORDER_PRIVATE_HPP
#define HEADER_RECORDER_PRIVATE_HPP

#include "openglrecorder.h"

#include <string>

extern ogrFucReadPixels ogrReadPixels;
extern ogrFucGenBuffers ogrGenBuffers;
extern ogrFucBindBuffer ogrBindBuffer;
extern ogrFucBufferData ogrBufferData;
extern ogrFucDeleteBuffers ogrDeleteBuffers;
extern ogrFucMapBuffer ogrMapBuffer;
extern ogrFucUnmapBuffer ogrUnmapBuffer;

RecorderConfig* getConfig();
const std::string& getSavedName();
void setCapturing(bool val);
void setThreadName(const char* name);
void runCallback(CallBackType cbt, const void* arg);

#endif

#endif
