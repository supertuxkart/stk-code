#ifndef APPLE_NETWORK_LIBRARIES

#ifdef WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <winsock2.h>
#endif
#include <curl/curl.h>

#if LIBCURL_VERSION_MAJOR > 7 || (LIBCURL_VERSION_MAJOR == 7 && LIBCURL_VERSION_MINOR > 31)
// Use CURLOPT_XFERINFOFUNCTION (introduced in 7.32.0)
#define PROGRESSDATA     CURLOPT_XFERINFODATA
#define PROGRESSFUNCTION CURLOPT_XFERINFOFUNCTION
typedef curl_off_t progress_t;
#else
// Use CURLOPT_PROGRESSFUNCTION (deprecated since 7.32.0)
#define PROGRESSDATA     CURLOPT_PROGRESSDATA
#define PROGRESSFUNCTION CURLOPT_PROGRESSFUNCTION
typedef double progress_t;
#endif

#include "online/http_request.hpp"
#include "io/file_manager.hpp"
#include "config/user_config.hpp"
#include "online/request_manager.hpp"
#include "utils/file_utils.hpp"
#include "utils/log.hpp"

// ============================================================================
bool Online::globalHTTPRequestInit()
{
    return curl_global_init(CURL_GLOBAL_DEFAULT) == CURLE_OK;
}   // globalHTTPRequestInit

// ============================================================================
void Online::globalHTTPRequestCleanup()
{
    curl_global_cleanup();
}   // globalHTTPRequestCleanup

// ----------------------------------------------------------------------------
/** Callback function from curl: inform about progress. It makes sure that
 *  the value reported by getProgress () is <1 while the download is still
 *  in progress.
 *  \param clientp
 *  \param download_total Total size of data to download.
 *  \param download_now   How much has been downloaded so far.
 *  \param upload_total   Total amount of upload.
 *  \param upload_now     How muc has been uploaded so far.
 */
int curlProgressDownload(void *clientp,
                         progress_t download_total, progress_t download_now,
                         progress_t upload_total,   progress_t upload_now)
{
    using namespace Online;
    HTTPRequest *request = (HTTPRequest *)clientp;

    // Check if we are asked to abort the download. If so, signal this
    // back to libcurl by returning a non-zero status.
    if (RequestManager::isRunning() &&
        (RequestManager::get()->getAbort() || RequestManager::get()->getPaused() ||
         request->isCancelled()) &&
         request->isAbortable()                                     )
    {
        // Indicates to abort the current download, which means that this
        // thread will go back to the mainloop and handle the next request.
        return 1;
    }

    float f;
    request->setTotalSize(download_total);
    if (download_now < download_total)
    {
        f = (float)download_now / (float)download_total;

        // In case of floating point rouding errors make sure that
        // 1.0 is only reached when downloadFileInternal is finished
        if (f >= 1.0f)
            f = 0.99f;
    }
    else
    {
        // Don't set progress to 1.0f; this is done in afterOperation()
        // after checking curls return code!
        f = (download_total == 0) ? 0 : 0.99f;
    }
    request->setProgress(f);

    return 0;
}   // progressDownload

// ----------------------------------------------------------------------------
/** Callback from curl. This stores the data received by curl in the
 *  buffer of this request.
 *  \param content Pointer to the data received by curl.
 *  \param size Size of one block.
 *  \param nmemb Number of blocks received.
 *  \param userp Pointer to the user buffer.
 */
size_t curlWriteCallback(void *contents, size_t size,
                         size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}   // curlWriteCallback

// ----------------------------------------------------------------------------
void Online::HTTPRequest::operation()
{
    CURL* curl_session = curl_easy_init();
    if (!curl_session)
    {
        Log::error("HTTPRequest::prepareOperation",
                   "LibCurl session not initialized.");
        m_result_code = CURLE_FAILED_INIT;
        return;
    }

    curl_easy_setopt(curl_session, CURLOPT_URL, m_url.c_str());
    curl_easy_setopt(curl_session, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_session, CURLOPT_NOPROGRESS, 0);
    curl_easy_setopt(curl_session, PROGRESSDATA, this);
    curl_easy_setopt(curl_session, PROGRESSFUNCTION,
                                   &curlProgressDownload);
    curl_easy_setopt(curl_session, CURLOPT_CONNECTTIMEOUT, 20);
    curl_easy_setopt(curl_session, CURLOPT_LOW_SPEED_LIMIT, 10);
    curl_easy_setopt(curl_session, CURLOPT_LOW_SPEED_TIME, 20);
    curl_easy_setopt(curl_session, CURLOPT_NOSIGNAL, 1);
    //curl_easy_setopt(curl_session, CURLOPT_VERBOSE, 1L);

    // https, load certificate info
    const std::string& ci = file_manager->getCertBundleLocation();
    CURLcode error = curl_easy_setopt(curl_session, CURLOPT_CAINFO, ci.c_str());
    if (error != CURLE_OK)
    {
        Log::error("HTTPRequest", "Error setting CAINFO to '%s'",
            ci.c_str());
        Log::error("HTTPRequest", "Error: '%s'.", error,
            curl_easy_strerror(error));
    }
    std::string host = "Host: " + StringUtils::getHostNameFromURL(m_url);
    struct curl_slist* http_header = NULL;
    http_header = curl_slist_append(http_header, host.c_str());
    assert(http_header != NULL);
    curl_easy_setopt(curl_session, CURLOPT_HTTPHEADER, http_header);
    curl_easy_setopt(curl_session, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl_session, CURLOPT_SSL_VERIFYHOST, 2L);

    FILE *fout = NULL;
    if (m_filename.size() > 0)
    {
        fout = FileUtils::fopenU8Path(m_filename + ".part", "wb");

        if (!fout)
        {
            Log::error("HTTPRequest",
                       "Can't open '%s' for writing, ignored.",
                       (m_filename+".part").c_str());
            return;
        }
        curl_easy_setopt(curl_session,  CURLOPT_WRITEDATA,     fout  );
        curl_easy_setopt(curl_session,  CURLOPT_WRITEFUNCTION, fwrite);
    }
    else
    {
        curl_easy_setopt(curl_session, CURLOPT_WRITEDATA,
                         &m_string_buffer);
        curl_easy_setopt(curl_session, CURLOPT_WRITEFUNCTION,
                         &curlWriteCallback);
    }

    // All parameters added have a '&' added
    if (m_parameters.size() > 0)
    {
        m_parameters.erase(m_parameters.size()-1);
    }

    if (m_parameters.size() == 0 && !m_disable_sending_log)
    {
        Log::info("HTTPRequest", "Downloading %s", m_url.c_str());
    }
    else if (Log::getLogLevel() <= Log::LL_INFO && !m_disable_sending_log)
    {
        logMessage();
    } // end log http request

    if (!m_download_assets_request)
    {
        curl_easy_setopt(curl_session, CURLOPT_POSTFIELDS,
            m_parameters.c_str());
    }
    const std::string& uagent = StringUtils::getUserAgentString();
    curl_easy_setopt(curl_session, CURLOPT_USERAGENT, uagent.c_str());

    m_result_code = curl_easy_perform(curl_session);
    Request::operation();

    if (fout)
    {
        fclose(fout);
        if (m_result_code == CURLE_OK)
        {
            if(UserConfigParams::logAddons())
                Log::info("HTTPRequest", "Download %s successfully.", m_filename.c_str());

            // The behaviour of rename is unspecified if the target
            // file should already exist - so remove it.
            bool ok = file_manager->removeFile(m_filename);
            if (!ok)
            {
                Log::error("addons",
                           "Could not removed existing addons.xml file.");
                m_result_code = CURLE_WRITE_ERROR;
            }
            int ret = FileUtils::renameU8Path(m_filename + ".part", m_filename);
            // In case of an error, set the status to indicate this
            if (ret != 0)
            {
                Log::error("addons",
                           "Could not rename downloaded addons.xml file!");
                m_result_code = CURLE_WRITE_ERROR;
            }
        }   // m_result_code ==CURLE_OK
    }   // if fout

    if (m_result_code != CURLE_OK)
    {
        Log::error("HTTPRequest", "Request failed with error code %lld: %s",
            m_result_code, getDownloadErrorMessage());
        setProgress(-1.0f);
    }
    else
        setProgress(1.0f);

    curl_slist_free_all(http_header);
    curl_easy_cleanup(curl_session);
}   // operation

// ----------------------------------------------------------------------------
const char* Online::HTTPRequest::getDownloadErrorMessage() const
{
    assert(hadDownloadError());
    return curl_easy_strerror((CURLcode)m_result_code);
}   // getDownloadErrorMessage

#endif
