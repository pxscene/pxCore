/*

 rtCore Copyright 2005-2017 John Robinson

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

*/

// rtFileDownloader.cpp

// TODO what is this for??
#define XRELOG_NOCTRACE

#include "rtFileDownloader.h"
#include "rtThreadTask.h"
#include "rtThreadPool.h"
#include "pxTimer.h"
#include "rtLog.h"
#ifdef ENABLE_ACCESS_CONTROL_CHECK
#include "rtCORSUtils.h"
#endif
#include <sstream>
#include <iostream>
#include <thread>
using namespace std;

#define CA_CERTIFICATE "cacert.pem"

//#define PX_REUSE_DOWNLOAD_HANDLES

const int kCurlTimeoutInSeconds = 30;
#ifdef PX_REUSE_DOWNLOAD_HANDLES
const int kMaxDownloadHandles = 6;
#endif //PX_REUSE_DOWNLOAD_HANDLES
const unsigned int kDefaultDownloadHandleExpiresTime = 5 * 60;
const int kDownloadHandleTimerIntervalInMilliSeconds = 30 * 1000;

std::thread* downloadHandleExpiresCheckThread = NULL;
bool continueDownloadHandleCheck = true;
rtMutex downloadHandleMutex;

#define HTTP_DOWNLOAD_CANCELED 499

struct MemoryStruct
{
    MemoryStruct()
        : headerSize(0)
        , headerBuffer(NULL)
        , contentsSize(0)
        , contentsBuffer(NULL)
        , downloadRequest(NULL)
    {
        headerBuffer = (char*)malloc(1);
        contentsBuffer = (char*)malloc(1);
    }

    ~MemoryStruct()
    {
      if (headerBuffer != NULL)
      {
        free(headerBuffer);
        headerBuffer = NULL;
      }
      if (contentsBuffer != NULL)
      {
        free(contentsBuffer);
        contentsBuffer = NULL;
      }
    }

  size_t headerSize;
  char* headerBuffer;
  size_t contentsSize;
  char* contentsBuffer;
  rtFileDownloadRequest *downloadRequest;
};

static size_t HeaderCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t downloadSize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  mem->headerBuffer = (char*)realloc(mem->headerBuffer, mem->headerSize + downloadSize + 1);
  if(mem->headerBuffer == NULL) {
    /* out of memory! */
    cout << "out of memory when downloading image\n";
    return 0;
  }

  memcpy(&(mem->headerBuffer[mem->headerSize]), contents, downloadSize);
  mem->headerSize += downloadSize;
  mem->headerBuffer[mem->headerSize] = 0;

  return downloadSize;
}

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t downloadSize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  mem->downloadRequest->executeDownloadProgressCallback(contents, size, nmemb );

  mem->contentsBuffer = (char*)realloc(mem->contentsBuffer, mem->contentsSize + downloadSize + 1);
  if(mem->contentsBuffer == NULL) {
    /* out of memory! */
    cout << "out of memory when downloading image\n";
    return 0;
  }

  memcpy(&(mem->contentsBuffer[mem->contentsSize]), contents, downloadSize);
  mem->contentsSize += downloadSize;
  mem->contentsBuffer[mem->contentsSize] = 0;

  return downloadSize;
}


void startFileDownloadInBackground(void* data)
{
    rtFileDownloadRequest* downloadRequest = (rtFileDownloadRequest*)data;
    rtFileDownloader::instance()->downloadFile(downloadRequest);
}

rtFileDownloader* rtFileDownloader::mInstance = NULL;
std::vector<rtFileDownloadRequest*>* rtFileDownloader::mDownloadRequestVector = new std::vector<rtFileDownloadRequest*>();
rtMutex* rtFileDownloader::mDownloadRequestVectorMutex = new rtMutex();


void onDownloadHandleCheck()
{
  rtLogDebug("inside onDownloadHandleCheck");
  bool checkHandles = true;
  while (checkHandles)
  {
	pxSleepMS(kDownloadHandleTimerIntervalInMilliSeconds);
    rtFileDownloader::instance()->checkForExpiredHandles();
    downloadHandleMutex.lock();
    checkHandles = continueDownloadHandleCheck;
    downloadHandleMutex.unlock();
  }
}

rtFileDownloadRequest::rtFileDownloadRequest(const char* imageUrl, void* callbackData, void (*callbackFunction)(rtFileDownloadRequest*))
      : mFileUrl(imageUrl), mProxyServer(),
    mErrorString(), mHttpStatusCode(0), mCallbackFunction(callbackFunction), mDownloadProgressCallbackFunction(NULL), mDownloadProgressUserPtr(NULL),
    mDownloadedData(0), mDownloadedDataSize(), mDownloadStatusCode(0) ,mCallbackData(callbackData),
    mCallbackFunctionMutex(), mHeaderData(0), mHeaderDataSize(0), mHeaderOnly(false), mDownloadHandleExpiresTime(-2)
#ifdef ENABLE_HTTP_CACHE
    , mCacheEnabled(true), mIsDataInCache(false), mDeferCacheRead(false)
#endif
    , mIsProgressMeterSwitchOff(false), mHTTPFailOnError(false), mDefaultTimeout(false)
    , mCanceled(false), mCanceledMutex()
{
  mAdditionalHttpHeaders.clear();
#ifdef ENABLE_HTTP_CACHE
  memset(mHttpErrorBuffer, 0, sizeof(mHttpErrorBuffer));
#endif
}

rtFileDownloadRequest::~rtFileDownloadRequest()
{
  if (mDownloadedData  != NULL)
  {
    free(mDownloadedData);
  }
  mDownloadedData = NULL;
  if (mHeaderData != NULL)
  {
    free(mHeaderData);
  }
  mHeaderData = NULL;
  mAdditionalHttpHeaders.clear();
  mHeaderOnly = false;
}

void rtFileDownloadRequest::setFileUrl(const char* imageUrl) { mFileUrl = imageUrl; }
rtString rtFileDownloadRequest::fileUrl() const { return mFileUrl; }

void rtFileDownloadRequest::setProxy(const char* proxyServer)
{
  mProxyServer = proxyServer;
}

rtString rtFileDownloadRequest::proxy() const
{
  return mProxyServer;
}

void rtFileDownloadRequest::setErrorString(const char* errorString)
{
  mErrorString = errorString;
}

rtString rtFileDownloadRequest::errorString()
{
  return mErrorString;
}

void rtFileDownloadRequest::setCallbackFunction(void (*callbackFunction)(rtFileDownloadRequest*))
{
  mCallbackFunction = callbackFunction;
}

void rtFileDownloadRequest::setDownloadProgressCallbackFunction(size_t (*callbackFunction)(void *ptr, size_t size, size_t nmemb, void *userData), void *userPtr)
{
  mDownloadProgressCallbackFunction = callbackFunction;
  mDownloadProgressUserPtr = userPtr;
}

void rtFileDownloadRequest::setCallbackFunctionThreadSafe(void (*callbackFunction)(rtFileDownloadRequest*))
{
  mCallbackFunctionMutex.lock();
  mCallbackFunction = callbackFunction;
  mCallbackFunctionMutex.unlock();
}

long rtFileDownloadRequest::httpStatusCode()
{
  return mHttpStatusCode;
}

void rtFileDownloadRequest::setHttpStatusCode(long statusCode)
{
  mHttpStatusCode = statusCode;
}

bool rtFileDownloadRequest::executeCallback(int statusCode)
{
  mDownloadStatusCode = statusCode;
  mCallbackFunctionMutex.lock();
  if (mCallbackFunction != NULL)
  {
    (*mCallbackFunction)(this);
    mCallbackFunctionMutex.unlock();
    return true;
  }
  mCallbackFunctionMutex.unlock();
  return false;
}

bool rtFileDownloadRequest::executeDownloadProgressCallback(void * ptr, size_t size, size_t nmemb)
{
  if(mDownloadProgressCallbackFunction)
  {
    mDownloadProgressCallbackFunction(ptr, size, nmemb, mDownloadProgressUserPtr);
    return true;
  }
  return false;
}

void rtFileDownloadRequest::setDownloadedData(char* data, size_t size)
{
  mDownloadedData = data;
  mDownloadedDataSize = size;
}

void rtFileDownloadRequest::downloadedData(char*& data, size_t& size)
{
  data = mDownloadedData;
  size = mDownloadedDataSize;
}

char* rtFileDownloadRequest::downloadedData()
{
  return mDownloadedData;
}

size_t rtFileDownloadRequest::downloadedDataSize()
{
  return mDownloadedDataSize;
}

void rtFileDownloadRequest::setHeaderData(char* data, size_t size)
{
  mHeaderData = data;
  mHeaderDataSize = size;
}

char* rtFileDownloadRequest::headerData()
{
  return mHeaderData;
}

size_t rtFileDownloadRequest::headerDataSize()
{
  return mHeaderDataSize;
}

/*  Function to set additional http headers */
void rtFileDownloadRequest::setAdditionalHttpHeaders(std::vector<rtString>& additionalHeaders)
{
  mAdditionalHttpHeaders = additionalHeaders;
}

std::vector<rtString>& rtFileDownloadRequest::additionalHttpHeaders()
{
  return mAdditionalHttpHeaders;
}

void rtFileDownloadRequest::setDownloadStatusCode(int statusCode)
{
  mDownloadStatusCode = statusCode;
}

int rtFileDownloadRequest::downloadStatusCode()
{
  return mDownloadStatusCode;
}

void* rtFileDownloadRequest::callbackData()
{
  return mCallbackData;
}

void rtFileDownloadRequest::setCallbackData(void* callbackData)
{
  mCallbackData = callbackData;
}

/* Function used to set to download only header or not */
void rtFileDownloadRequest::setHeaderOnly(bool val)
{
  mHeaderOnly = val;
}

bool rtFileDownloadRequest::headerOnly()
{
  return mHeaderOnly;
}

void rtFileDownloadRequest::setDownloadHandleExpiresTime(int timeInSeconds)
{
  mDownloadHandleExpiresTime = timeInSeconds;
}

int rtFileDownloadRequest::downloadHandleExpiresTime()
{
  return mDownloadHandleExpiresTime;
}

#ifdef ENABLE_HTTP_CACHE
/* Function used to enable or disable using file cache */
void rtFileDownloadRequest::setCacheEnabled(bool val)
{
  mCacheEnabled = val;
}

bool rtFileDownloadRequest::cacheEnabled()
{
  return mCacheEnabled;
}

void rtFileDownloadRequest::setDataIsCached(bool val)
{
  mIsDataInCache = val;
}

bool rtFileDownloadRequest::isDataCached()
{
  return mIsDataInCache;
}

void rtFileDownloadRequest::setDeferCacheRead(bool val)
{
  mDeferCacheRead = val;
}

bool rtFileDownloadRequest::deferCacheRead()
{
  return mDeferCacheRead;
}

FILE* rtFileDownloadRequest::cacheFilePointer(void)
{
  rtHttpCacheData cachedData(this->fileUrl().cString());

  if (true == this->cacheEnabled())
  {
    if ((NULL != rtFileCache::instance()) && (RT_OK == rtFileCache::instance()->httpCacheData(this->fileUrl(), cachedData)))
    {
      return cachedData.filePointer();
    }
  }
  return NULL;
}
#endif //ENABLE_HTTP_CACHE

void rtFileDownloadRequest::setProgressMeter(bool val)
{
  mIsProgressMeterSwitchOff = val;
}

bool rtFileDownloadRequest::isProgressMeterSwitchOff()
{
  return mIsProgressMeterSwitchOff;
}

void rtFileDownloadRequest::setHTTPFailOnError(bool val)
{
  mHTTPFailOnError = val;
}

bool rtFileDownloadRequest::isHTTPFailOnError()
{
  return mHTTPFailOnError;
}

void rtFileDownloadRequest::setHTTPError(const char* httpError)
{
  if(httpError != NULL)
  {
    strncpy(mHttpErrorBuffer, httpError, CURL_ERROR_SIZE);
    mHttpErrorBuffer[CURL_ERROR_SIZE-1] = '\0';
  }
}

char* rtFileDownloadRequest::httpErrorBuffer(void)
{
  return mHttpErrorBuffer;
}

void rtFileDownloadRequest::setCurlDefaultTimeout(bool val)
{
  mDefaultTimeout = val;
}

bool rtFileDownloadRequest::isCurlDefaultTimeoutSet()
{
  return mDefaultTimeout;
}

void rtFileDownloadRequest::setOrigin(const char* origin)
{
  mOrigin = origin;
}

rtString rtFileDownloadRequest::origin()
{
  return mOrigin;
}

void rtFileDownloadRequest::cancelRequest()
{
  mCanceledMutex.lock();
  mCanceled = true;
  mCanceledMutex.unlock();
}

bool rtFileDownloadRequest::isCanceled()
{
  bool requestCanceled = false;
  mCanceledMutex.lock();
  requestCanceled = mCanceled;
  mCanceledMutex.unlock();
  return requestCanceled;
}

rtFileDownloader::rtFileDownloader()
    : mNumberOfCurrentDownloads(0), mDefaultCallbackFunction(NULL), mDownloadHandles(), mReuseDownloadHandles(false),
      mCaCertFile(CA_CERTIFICATE), mFileCacheMutex()
{
#ifdef PX_REUSE_DOWNLOAD_HANDLES
  rtLogWarn("enabling curl handle reuse");
  for (int i = 0; i < kMaxDownloadHandles; i++)
  {
    mDownloadHandles.push_back(rtFileDownloadHandle(curl_easy_init()));
  }
  mReuseDownloadHandles = true;
#endif
  char const* s = getenv("CA_CERTIFICATE_FILE");
  if (s)
  {
    mCaCertFile = s;
  }
}

rtFileDownloader::~rtFileDownloader()
{
#ifdef PX_REUSE_DOWNLOAD_HANDLES
  downloadHandleMutex.lock();
  for (vector<rtFileDownloadHandle>::iterator it = mDownloadHandles.begin(); it != mDownloadHandles.end();++it)
  {
    CURL* curlHandle = (*it).curlHandle;
    if (curlHandle != NULL)
    {
      curl_easy_cleanup(curlHandle);
    }
    it = mDownloadHandles.erase(it);
  }
  mReuseDownloadHandles = false;
  downloadHandleMutex.unlock();
  if (rtFileDownloader::instance() == this)
  {
    //cleanup curl and shutdown the reuse handle thread if this is the singleton object
    downloadHandleMutex.lock();
    continueDownloadHandleCheck = false;
    downloadHandleMutex.unlock();
    if (downloadHandleExpiresCheckThread)
    {
      rtLogDebug("close thread and wait");
      downloadHandleExpiresCheckThread->join();
      rtLogDebug("done with join");
      delete downloadHandleExpiresCheckThread;
      downloadHandleExpiresCheckThread = NULL;
    }
  }
#endif
  mCaCertFile = "";
}

rtFileDownloader* rtFileDownloader::instance()
{
    if (mInstance == NULL)
    {
        mInstance = new rtFileDownloader();
#ifdef PX_REUSE_DOWNLOAD_HANDLES
        downloadHandleExpiresCheckThread = new std::thread(onDownloadHandleCheck);
#endif //PX_REUSE_DOWNLOAD_HANDLES
    }
    return mInstance;
}

bool rtFileDownloader::addToDownloadQueue(rtFileDownloadRequest* downloadRequest)
{
    bool submitted = false;
    //todo: check the download queue before starting download
    submitted = true;
    addFileDownloadRequest(downloadRequest);
    downloadFileInBackground(downloadRequest);
    //startNextDownloadInBackground();
    return submitted;
}

void rtFileDownloader::startNextDownloadInBackground()
{
    //todo
}

void rtFileDownloader::raiseDownloadPriority(rtFileDownloadRequest* downloadRequest)
{
  if (downloadRequest != NULL)
  {
    rtThreadPool *mainThreadPool = rtThreadPool::globalInstance();
    mainThreadPool->raisePriority(downloadRequest->fileUrl());
  }
}

void rtFileDownloader::removeDownloadRequest(rtFileDownloadRequest* downloadRequest)
{
    (void)downloadRequest;
    //todo
}

void rtFileDownloader::clearFileCache()
{
    //todo
}

void rtFileDownloader::downloadFile(rtFileDownloadRequest* downloadRequest)
{
  bool isRequestCanceled = downloadRequest->isCanceled();
  if (isRequestCanceled)
  {
    downloadRequest->setDownloadStatusCode(HTTP_DOWNLOAD_CANCELED);
    downloadRequest->setDownloadedData(NULL, 0);
    downloadRequest->setDownloadStatusCode(-1);
    downloadRequest->setErrorString("canceled request");
    if (!downloadRequest->executeCallback(downloadRequest->downloadStatusCode()))
    {
      if (mDefaultCallbackFunction != NULL)
      {
        (*mDefaultCallbackFunction)(downloadRequest);
      }
    }
    clearFileDownloadRequest(downloadRequest);
    return;
  }

#ifdef ENABLE_HTTP_CACHE
    bool isDataInCache = false;
#endif
    bool nwDownloadSuccess = false;

#ifdef ENABLE_HTTP_CACHE
    rtHttpCacheData cachedData(downloadRequest->fileUrl().cString());
    if (true == downloadRequest->cacheEnabled())
    {
      if (true == checkAndDownloadFromCache(downloadRequest,cachedData))
      {
        isDataInCache = true;
        downloadRequest->setDataIsCached(true);
      }
    }

    if (false == isDataInCache)
#endif
    {
      nwDownloadSuccess = downloadFromNetwork(downloadRequest);
    }

    if (!downloadRequest->executeCallback(downloadRequest->downloadStatusCode()))
    {
      if (mDefaultCallbackFunction != NULL)
      {
        (*mDefaultCallbackFunction)(downloadRequest);
      }
    }

#ifdef ENABLE_HTTP_CACHE
    // Store the network data in cache
    if ((true == nwDownloadSuccess) &&
        (true == downloadRequest->cacheEnabled())  &&
        (downloadRequest->httpStatusCode() != 206) &&
        (downloadRequest->httpStatusCode() != 302) &&
        (downloadRequest->httpStatusCode() != 307))
    {
      rtHttpCacheData downloadedData(downloadRequest->fileUrl(),
                                     downloadRequest->headerData(),
                                     downloadRequest->downloadedData(),
                                     downloadRequest->downloadedDataSize());

      if (downloadedData.isWritableToCache())
      {
        mFileCacheMutex.lock();
        if (NULL == rtFileCache::instance())
          rtLogWarn("cache data not added");
        else
        {
          rtFileCache::instance()->addToCache(downloadedData);
          rtLogInfo("Cache expiration(%s)", cachedData.expirationDate().cString());
        }
        mFileCacheMutex.unlock();
      }
    }

    // Store the updated data in cache
    if ((true == isDataInCache) && (cachedData.isUpdated()))
    {
      rtString url;
      cachedData.url(url);

      mFileCacheMutex.lock();
      if (NULL == rtFileCache::instance())
          rtLogWarn("Adding url to cache failed (%s) due to in-process memory issues", url.cString());
      rtFileCache::instance()->removeData(url);
      mFileCacheMutex.unlock();
      if (cachedData.isWritableToCache())
      {
        mFileCacheMutex.lock();
        rtError err = rtFileCache::instance()->addToCache(cachedData);
        mFileCacheMutex.unlock();
        if (RT_OK != err)
          rtLogWarn("Adding url to cache failed (%s)", url.cString());
        else
          rtLogInfo("Cache expiration(%s)", cachedData.expirationDate().cString());
      }
    }

    if (true == isDataInCache)
    {
      downloadRequest->setHeaderData(NULL,0);
      downloadRequest->setDownloadedData(NULL,0);
    }
#endif
    clearFileDownloadRequest(downloadRequest);
}

bool rtFileDownloader::downloadFromNetwork(rtFileDownloadRequest* downloadRequest)
{
    CURL *curl_handle = NULL;
    CURLcode res = CURLE_OK;
    char errorBuffer[CURL_ERROR_SIZE];

    bool useProxy = !downloadRequest->proxy().isEmpty();
    rtString proxyServer = downloadRequest->proxy();
    bool headerOnly = downloadRequest->headerOnly();
    MemoryStruct chunk;

    curl_handle = rtFileDownloader::instance()->retrieveDownloadHandle();

    /* specify URL to get */
    curl_easy_setopt(curl_handle, CURLOPT_URL, downloadRequest->fileUrl().cString());
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1); //when redirected, follow the redirections
    curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, HeaderCallback);
    curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, (void *)&chunk);
    if (false == headerOnly)
    {
      chunk.downloadRequest = downloadRequest;
      curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
      curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    }

    if(downloadRequest->isCurlDefaultTimeoutSet() == false)
    {
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, kCurlTimeoutInSeconds);
    curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1);
    }

    if(downloadRequest->isProgressMeterSwitchOff())
        curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1);

    if(downloadRequest->isHTTPFailOnError())
    {
        memset(errorBuffer, 0, sizeof(errorBuffer));
        curl_easy_setopt(curl_handle, CURLOPT_FAILONERROR, 1);
        curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1);
        curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, errorBuffer);
    }
#if !defined(PX_PLATFORM_GENERIC_DFB) && !defined(PX_PLATFORM_DFB_NON_X11)
    curl_easy_setopt(curl_handle, CURLOPT_TCP_KEEPALIVE, 1);
    curl_easy_setopt(curl_handle, CURLOPT_TCP_KEEPIDLE, 60);
    curl_easy_setopt(curl_handle, CURLOPT_TCP_KEEPINTVL, 30);
#endif //!PX_PLATFORM_GENERIC_DFB && !PX_PLATFORM_DFB_NON_X11

    int downloadHandleExpiresTime = downloadRequest->downloadHandleExpiresTime();

    vector<rtString>& additionalHttpHeaders = downloadRequest->additionalHttpHeaders();
    struct curl_slist *list = NULL;
    for (unsigned int headerOption = 0;headerOption < additionalHttpHeaders.size();headerOption++)
    {
      list = curl_slist_append(list, additionalHttpHeaders[headerOption].cString());
    }
#ifdef ENABLE_ACCESS_CONTROL_CHECK
    const rtString& origin = downloadRequest->origin();
    if (!origin.isEmpty())
    {
      rtString headerOrigin("Origin:");
      headerOrigin.append(origin.cString());
      list = curl_slist_append(list, headerOrigin.cString());
    }
#endif
    curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, list);
    //CA certificates
    // !CLF: Use system CA Cert rather than CA_CERTIFICATE fo now.  Revisit!
    //curl_easy_setopt(curl_handle,CURLOPT_CAINFO,mCaCertFile.cString());
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 2);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, true);

    /* some servers don't like requests that are made without a user-agent
     field, so we provide one */
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    if (useProxy)

    {
        curl_easy_setopt(curl_handle, CURLOPT_PROXY, proxyServer.cString());
        curl_easy_setopt(curl_handle, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
    }
    else
    {
      curl_easy_setopt(curl_handle, CURLOPT_PROXY, "");
    }

    if (true == headerOnly)
    {
      curl_easy_setopt(curl_handle, CURLOPT_NOBODY, 1);
    }
    /* get it! */
    res = curl_easy_perform(curl_handle);
    downloadRequest->setDownloadStatusCode(res);
    if(downloadRequest->isHTTPFailOnError())
        downloadRequest->setHTTPError(errorBuffer);

    /* check for errors */
    if (res != CURLE_OK)
    {
        stringstream errorStringStream;

        errorStringStream << "Download error for: " << downloadRequest->fileUrl().cString()
                << ".  Error code : " << res << ".  Using proxy: ";
        if (useProxy)
        {
            errorStringStream << "true - " << proxyServer.cString();
        }
        else
        {
            errorStringStream << "false";
        }

        downloadRequest->setErrorString(errorStringStream.str().c_str());
        rtFileDownloader::instance()->releaseDownloadHandle(curl_handle, downloadHandleExpiresTime);

        //clean up contents on error
        if (chunk.contentsBuffer != NULL)
        {
            free(chunk.contentsBuffer);
            chunk.contentsBuffer = NULL;
        }

        if (chunk.headerBuffer != NULL)
        {
            free(chunk.headerBuffer);
            chunk.headerBuffer = NULL;
        }
        downloadRequest->setDownloadedData(NULL, 0);
        return false;
    }

    long httpCode = -1;
    if (curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &httpCode) == CURLE_OK)
    {
        downloadRequest->setHttpStatusCode(httpCode);
    }
    curl_slist_free_all(list);
    rtFileDownloader::instance()->releaseDownloadHandle(curl_handle, downloadHandleExpiresTime);

    //todo read the header information before closing
    if (chunk.headerBuffer != NULL)
    {
        downloadRequest->setHeaderData(chunk.headerBuffer, chunk.headerSize);
    }

    //don't free the downloaded data (contentsBuffer) because it will be used later
    if (false == headerOnly)
    {
      downloadRequest->setDownloadedData(chunk.contentsBuffer, chunk.contentsSize);
#ifdef ENABLE_ACCESS_CONTROL_CHECK
      rtString errorStr;
      rtString rawHeaders(downloadRequest->headerData(), downloadRequest->headerDataSize());
      if (RT_OK != rtCORSUtilsCheckOrigin(origin, downloadRequest->fileUrl(), rawHeaders, &errorStr))
      {
        rtLogWarn("disallow access for origin '%s' because: %s", origin.cString(), errorStr.cString());

        // Disallow access to the resource's contents.
        if (downloadRequest->downloadedData() != NULL)
        {
          free(downloadRequest->downloadedData());
        }
        downloadRequest->setDownloadedData(NULL, 0);
        downloadRequest->setDownloadStatusCode(-1);
        downloadRequest->setErrorString(errorStr.cString());
      }
#endif
    }
    else if (chunk.contentsBuffer != NULL)
    {
        free(chunk.contentsBuffer);
        chunk.contentsBuffer = NULL;
    }
    chunk.headerBuffer = NULL;
    chunk.contentsBuffer = NULL;
    return true;
}

#ifdef ENABLE_HTTP_CACHE
bool rtFileDownloader::checkAndDownloadFromCache(rtFileDownloadRequest* downloadRequest,rtHttpCacheData& cachedData)
{
  rtError err;
  rtData data;
  mFileCacheMutex.lock();
  if ((NULL != rtFileCache::instance()) && (RT_OK == rtFileCache::instance()->httpCacheData(downloadRequest->fileUrl(),cachedData)))
  {
    if(downloadRequest->deferCacheRead())
      err = cachedData.deferCacheRead(data);
    else
      err = cachedData.data(data);
    if (RT_OK !=  err)
    {
      mFileCacheMutex.unlock();
      return false;
    }

    downloadRequest->setHeaderData((char *)cachedData.headerData().data(),cachedData.headerData().length());
    downloadRequest->setDownloadedData((char *)cachedData.contentsData().data(),cachedData.contentsData().length());
    downloadRequest->setDownloadStatusCode(0);
    downloadRequest->setHttpStatusCode(200);
    mFileCacheMutex.unlock();
    return true;
  }
  mFileCacheMutex.unlock();
  return false;
}
#endif

void rtFileDownloader::downloadFileInBackground(rtFileDownloadRequest* downloadRequest)
{
    rtThreadPool* mainThreadPool = rtThreadPool::globalInstance();

    if (downloadRequest->downloadHandleExpiresTime() < -1)
    {
      downloadRequest->setDownloadHandleExpiresTime(kDefaultDownloadHandleExpiresTime);
    }

    rtThreadTask* task = new rtThreadTask(startFileDownloadInBackground, (void*)downloadRequest, downloadRequest->fileUrl());

    mainThreadPool->executeTask(task);
}

rtFileDownloadRequest* rtFileDownloader::nextDownloadRequest()
{
    //todo
    return NULL;
}

void rtFileDownloader::setDefaultCallbackFunction(void (*callbackFunction)(rtFileDownloadRequest*))
{
  mDefaultCallbackFunction = callbackFunction;
}

CURL* rtFileDownloader::retrieveDownloadHandle()
{
  CURL* curlHandle = NULL;
#ifdef PX_REUSE_DOWNLOAD_HANDLES
  downloadHandleMutex.lock();
  if (!mReuseDownloadHandles || mDownloadHandles.empty())
  {
    curlHandle = curl_easy_init();
  }
  else
  {
    curlHandle = mDownloadHandles.back().curlHandle;
    mDownloadHandles.pop_back();
  }
  downloadHandleMutex.unlock();
#else
  curlHandle = curl_easy_init();
#endif //PX_REUSE_DOWNLOAD_HANDLES
  if (curlHandle == NULL)
  {
    curlHandle = curl_easy_init();
  }
  return curlHandle;
}

void rtFileDownloader::releaseDownloadHandle(CURL* curlHandle, int expiresTime)
{
  rtLogDebug("expires time: %d", expiresTime);
#ifdef PX_REUSE_DOWNLOAD_HANDLES
    downloadHandleMutex.lock();
    if(!mReuseDownloadHandles || mDownloadHandles.size() >= kMaxDownloadHandles || (expiresTime == 0))
    {
      curl_easy_cleanup(curlHandle);
    }
    else
    {
        if (expiresTime > 0)
        {
          expiresTime += (int)pxSeconds();
        }
    	mDownloadHandles.push_back(rtFileDownloadHandle(curlHandle, expiresTime));
    }
    downloadHandleMutex.unlock();
#else
    curl_easy_cleanup(curlHandle);
#endif //PX_REUSE_DOWNLOAD_HANDLES
}

void rtFileDownloader::addFileDownloadRequest(rtFileDownloadRequest* downloadRequest)
{
  if (downloadRequest == NULL)
  {
    return;
  }
  mDownloadRequestVectorMutex->lock();
  bool found = false;
  for (std::vector<rtFileDownloadRequest*>::iterator it=mDownloadRequestVector->begin(); it!=mDownloadRequestVector->end(); ++it)
  {
    if ((*it) == downloadRequest)
    {
      found = true;
      break;
    }
  }
  if (!found)
  {
    mDownloadRequestVector->push_back(downloadRequest);
  }
  mDownloadRequestVectorMutex->unlock();
}

void rtFileDownloader::clearFileDownloadRequest(rtFileDownloadRequest* downloadRequest)
{
  mDownloadRequestVectorMutex->lock();
  for (std::vector<rtFileDownloadRequest*>::iterator it=mDownloadRequestVector->begin(); it!=mDownloadRequestVector->end(); ++it)
  {
    if ((*it) == downloadRequest)
    {
      mDownloadRequestVector->erase(it);
      break;
    }
  }
  if (downloadRequest != NULL)
  {
    delete downloadRequest;
  }
  mDownloadRequestVectorMutex->unlock();
}

void rtFileDownloader::setCallbackFunctionThreadSafe(rtFileDownloadRequest* downloadRequest,
                                                     void (*callbackFunction)(rtFileDownloadRequest*), void* owner)
{
  mDownloadRequestVectorMutex->lock();
  for (std::vector<rtFileDownloadRequest*>::iterator it=mDownloadRequestVector->begin(); it!=mDownloadRequestVector->end(); ++it)
  {
    if ((*it) == downloadRequest && (*it)->callbackData() == owner)
    {
      downloadRequest->setCallbackFunctionThreadSafe(callbackFunction);
      break;
    }
  }
  mDownloadRequestVectorMutex->unlock();
}

void rtFileDownloader::cancelDownloadRequestThreadSafe(rtFileDownloadRequest* downloadRequest, void* owner)
{
  mDownloadRequestVectorMutex->lock();
  for (std::vector<rtFileDownloadRequest*>::iterator it=mDownloadRequestVector->begin(); it!=mDownloadRequestVector->end(); ++it)
  {
    if ((*it) == downloadRequest && (*it)->callbackData() == owner)
    {
      downloadRequest->cancelRequest();
      break;
    }
  }
  mDownloadRequestVectorMutex->unlock();
}

bool rtFileDownloader::isDownloadRequestCanceled(rtFileDownloadRequest* downloadRequest, void* owner)
{
  bool requestIsCanceled = false;
  mDownloadRequestVectorMutex->lock();
  for (std::vector<rtFileDownloadRequest*>::iterator it=mDownloadRequestVector->begin(); it!=mDownloadRequestVector->end(); ++it)
  {
    if ((*it) == downloadRequest && (*it)->callbackData() == owner)
    {
      requestIsCanceled = downloadRequest->isCanceled();
      break;
    }
  }
  mDownloadRequestVectorMutex->unlock();
  return requestIsCanceled;
}

void rtFileDownloader::checkForExpiredHandles()
{
  rtLogDebug("inside checkForExpiredHandles");
  downloadHandleMutex.lock();
  for (vector<rtFileDownloadHandle>::iterator it = mDownloadHandles.begin(); it != mDownloadHandles.end();)
  {
    rtFileDownloadHandle fileDownloadHandle = (*it);
    rtLogDebug("expires time: %d\n", fileDownloadHandle.expiresTime);
    if (fileDownloadHandle.expiresTime < 0)
    {
      ++it;
      continue;
    }
    else if (pxSeconds() > fileDownloadHandle.expiresTime)
    {
      rtLogDebug("erasing handle!!!\n");
      curl_easy_cleanup(fileDownloadHandle.curlHandle);
      it = mDownloadHandles.erase(it);
    }
    else
    {
      ++it;
    }
  }
  downloadHandleMutex.unlock();
}
