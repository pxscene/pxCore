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

// rtFileDownloader.h

#ifndef RT_FILE_DOWNLOADER_H
#define RT_FILE_DOWNLOADER_H

#include "rtCore.h"
#include "rtString.h"
#ifdef ENABLE_HTTP_CACHE
#include <rtFileCache.h>
#endif

// TODO Eliminate std::string
#include <string.h>
#include <vector>

#include <curl/curl.h>

class rtFileDownloadRequest
{
public:
   rtFileDownloadRequest(const char* imageUrl, void* callbackData);
  ~rtFileDownloadRequest();

  void setFileUrl(const char* imageUrl);
  rtString fileUrl() const;
  void setProxy(const char* proxyServer);
  rtString proxy() const;
  void setErrorString(const char* errorString);
  rtString errorString();
  void setCallbackFunction(void (*callbackFunction)(rtFileDownloadRequest*));
  void setCallbackFunctionThreadSafe(void (*callbackFunction)(rtFileDownloadRequest*));
  long httpStatusCode();
  void setHttpStatusCode(long statusCode);
  bool executeCallback(int statusCode);
  void setDownloadedData(char* data, size_t size);
  void downloadedData(char*& data, size_t& size);
  char* downloadedData();
  size_t downloadedDataSize();
  void setHeaderData(char* data, size_t size);
  char* headerData();
  size_t headerDataSize();
  void setAdditionalHttpHeaders(std::vector<rtString>& additionalHeaders);
  std::vector<rtString>& additionalHttpHeaders();
  void setDownloadStatusCode(int statusCode);
  int downloadStatusCode();
  void* callbackData();
  void setCallbackData(void* callbackData);
  void setHeaderOnly(bool val);
  bool headerOnly();
  void setDownloadHandleExpiresTime(int timeInSeconds);
  int downloadHandleExpiresTime();
#ifdef ENABLE_HTTP_CACHE
  void setCacheEnabled(bool val);
  bool cacheEnabled();
#endif

private:
  rtString mFileUrl;
  rtString mProxyServer;
  rtString mErrorString;
  long mHttpStatusCode;
  void (*mCallbackFunction)(rtFileDownloadRequest*);
  char* mDownloadedData;
  size_t mDownloadedDataSize;
  int mDownloadStatusCode;
  void* mCallbackData;
  rtMutex mCallbackFunctionMutex;
  char* mHeaderData;
  size_t mHeaderDataSize;
  std::vector<rtString> mAdditionalHttpHeaders;
  bool mHeaderOnly;
  int mDownloadHandleExpiresTime;
#ifdef ENABLE_HTTP_CACHE
  bool mCacheEnabled;
#endif
};

struct rtFileDownloadHandle
{
  rtFileDownloadHandle(CURL* handle) : curlHandle(handle), expiresTime(-1) {}
  rtFileDownloadHandle(CURL* handle, int time) : curlHandle(handle), expiresTime(time) {}
  CURL* curlHandle;
  int expiresTime;
};

class rtFileDownloader
{
public:

    static rtFileDownloader* instance();

    virtual bool addToDownloadQueue(rtFileDownloadRequest* downloadRequest);
    virtual void raiseDownloadPriority(rtFileDownloadRequest* downloadRequest);
    virtual void removeDownloadRequest(rtFileDownloadRequest* downloadRequest);

    void clearFileCache();
    void downloadFile(rtFileDownloadRequest* downloadRequest);
    void setDefaultCallbackFunction(void (*callbackFunction)(rtFileDownloadRequest*));
    bool downloadFromNetwork(rtFileDownloadRequest* downloadRequest);
    void checkForExpiredHandles();

private:
    rtFileDownloader();
    ~rtFileDownloader();

    void startNextDownload(rtFileDownloadRequest* downloadRequest);
    rtFileDownloadRequest* nextDownloadRequest();
    void startNextDownloadInBackground();
    void downloadFileInBackground(rtFileDownloadRequest* downloadRequest);
#ifdef ENABLE_HTTP_CACHE
    bool checkAndDownloadFromCache(rtFileDownloadRequest* downloadRequest,rtHttpCacheData& cachedData);
#endif
    CURL* retrieveDownloadHandle();
    void releaseDownloadHandle(CURL* curlHandle, int expiresTime);
    //todo: hash mPendingDownloadRequests;
    //todo: string list mPendingDownloadOrderList;
    //todo: list mActiveDownloads;
    unsigned int mNumberOfCurrentDownloads;
    //todo: hash m_priorityDownloads;
    void (*mDefaultCallbackFunction)(rtFileDownloadRequest*);
    std::vector<rtFileDownloadHandle> mDownloadHandles;
    bool mReuseDownloadHandles;
    
    static rtFileDownloader* mInstance;
};

#endif //RT_FILE_DOWNLOADER_H
