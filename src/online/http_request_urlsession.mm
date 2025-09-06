#ifdef APPLE_NETWORK_LIBRARIES

#include "online/http_request.hpp"
#include "config/user_config.hpp"
#include "online/request_manager.hpp"
#include "utils/log.hpp"

#import <Foundation/Foundation.h>
#include <algorithm>
#include <cmath>

// ============================================================================
// NSURLSession doesn't require global initialization like cURL
bool Online::globalHTTPRequestInit()
{
    return true;
}   // globalHTTPRequestInit

// ============================================================================
void Online::globalHTTPRequestCleanup()
{
}   // globalHTTPRequestCleanup

// ----------------------------------------------------------------------------
void Online::HTTPRequest::operation()
{
    NSString *urlString = [NSString stringWithUTF8String:m_url.c_str()];
    NSURL *url = [NSURL URLWithString:urlString];
    if (!url)
    {
        m_result_code = kCFErrorHTTPBadURL;
        m_error_string = "Invalid URL: " + m_url;
        return;
    }

    NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:url];

    // Set user agent
    const std::string& uagent = StringUtils::getUserAgentString();
    NSString *userAgent = [NSString stringWithUTF8String:uagent.c_str()];
    [request setValue:userAgent forHTTPHeaderField:@"User-Agent"];

    // All parameters added have a '&' added
    if (m_parameters.size() > 0)
    {
        m_parameters.erase(m_parameters.size() - 1);
    }

    if (m_parameters.size() == 0 && !m_disable_sending_log)
    {
        Log::info("HTTPRequest", "Downloading %s", m_url.c_str());
    }
    else if (Log::getLogLevel() <= Log::LL_INFO && !m_disable_sending_log)
    {
        logMessage();
    }

    // Configure request method and body
    if (!m_download_assets_request && m_parameters.size() > 0)
    {
        [request setHTTPMethod:@"POST"];
        NSString *paramString = [NSString stringWithUTF8String:m_parameters.c_str()];
        NSData *postData = [paramString dataUsingEncoding:NSUTF8StringEncoding];
        [request setHTTPBody:postData];
        [request setValue:@"application/x-www-form-urlencoded" forHTTPHeaderField:@"Content-Type"];
    }
    else
    {
        [request setHTTPMethod:@"GET"];
    }

    __block HTTPRequest* blockSelf = this;
    __block NSString* blockFilename = nil;
    std::string temp_file;
    if (m_filename.size() > 0)
    {
        temp_file = m_filename + ".part";
        blockFilename = [NSString stringWithUTF8String:temp_file.c_str()];
    }

    NSURLSessionConfiguration *config = [NSURLSessionConfiguration defaultSessionConfiguration];
    config.timeoutIntervalForRequest = 20.0;
    config.timeoutIntervalForResource = 60.0;

    NSURLSession *session = [NSURLSession sessionWithConfiguration:config];
    std::atomic_bool completed(false);
    __block std::atomic_bool* completed_ptr = &completed;

    NSURLSessionDataTask *task = [session dataTaskWithRequest:request
                                 completionHandler:^(NSData *data, NSURLResponse *response, NSError *error)
    {
        if (error)
        {
            blockSelf->m_result_code = [error code];
            blockSelf->m_error_string = [[error localizedDescription] UTF8String];
        }
        else
        {
            blockSelf->m_result_code = 0;
            if (blockFilename)
            {
                // Save to file
                BOOL success = [data writeToFile:blockFilename atomically:YES];
                if (success)
                {
                    if (UserConfigParams::logAddons())
                        Log::info("HTTPRequest", "Download %s successfully.", m_filename.c_str());

                    // Remove existing file and rename
                    NSString *finalFilename = [NSString stringWithUTF8String:blockSelf->m_filename.c_str()];
                    NSFileManager *fileManager = [NSFileManager defaultManager];

                    NSError *removeError;
                    [fileManager removeItemAtPath:finalFilename error:&removeError];

                    NSError *moveError;
                    success = [fileManager moveItemAtPath:blockFilename toPath:finalFilename error:&moveError];
                    if (!success)
                    {
                        blockSelf->m_error_string = "Could not rename downloaded file: ";
                        blockSelf->m_error_string += [[moveError localizedDescription] UTF8String];
                        blockSelf->m_result_code = kCFHostErrorUnknown;
                    }
                }
                else
                {
                    blockSelf->m_error_string = "Could not write to file: " + temp_file;
                    blockSelf->m_result_code = kCFHostErrorUnknown;
                }
            }
            else
            {
                // Save to string buffer
                if (data)
                {
                    const char *bytes = (const char*)[data bytes];
                    NSUInteger length = [data length];
                    blockSelf->m_string_buffer = std::string(bytes, length);
                }
            }
        }
        [session finishTasksAndInvalidate];
        completed_ptr->store(true);
    }];

    [task resume];

    // Wait for completion (synchronous operation)
    bool cancelled = false;
    while (!completed.load())
    {
        if (!cancelled && task.state == NSURLSessionTaskStateRunning)
        {
            int64_t bytesReceived = task.countOfBytesReceived;
            int64_t bytesExpected = task.countOfBytesExpectedToReceive;
            if (bytesExpected > 0)
            {
                setTotalSize((double)bytesExpected);
                float progress = float((double)bytesReceived / (double)bytesExpected);
                if (progress >= 1.0f)
                    progress = 0.99f;
                setProgress(progress);
            }
            else if (bytesReceived > 0)
            {
                // We're receiving data but don't know the total size
                // Use a slow-growing progress that never reaches 1.0
                float unknownProgress = 1.0f - exp(-bytesReceived / 1024.0f / 1024.0f); // Asymptotic to 1.0
                unknownProgress = std::min(unknownProgress, 0.99f); // Cap at 99%
                setProgress(unknownProgress);
            }
        }
        if (!cancelled && RequestManager::isRunning() &&
            (RequestManager::get()->getAbort() ||
            RequestManager::get()->getPaused() || isCancelled()) &&
            isAbortable())
        {
            [task cancel];
            cancelled = true;
            continue;
        }

        // Brief sleep to avoid busy waiting
        usleep(10000); // 10ms
    }
    if (m_result_code != 0)
    {
        Log::error("HTTPRequest", "Request failed with error code %lld: %s",
            m_result_code, getDownloadErrorMessage());
        setProgress(-1.0f);
    }
    else
        setProgress(1.0f);
    Request::operation();
}   // operation

// ----------------------------------------------------------------------------
const char* Online::HTTPRequest::getDownloadErrorMessage() const
{
    assert(hadDownloadError());
    return m_error_string.c_str();
}   // getDownloadErrorMessage

#endif
