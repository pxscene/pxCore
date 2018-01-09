'use strict';

var https = require('https');

function isLocalAccess(reqOptions)
{
  if (((reqOptions.hostname) && ((reqOptions.hostname === "localhost") || (reqOptions.hostname === "127.0.0.1"))) || ((reqOptions.host) && ((reqOptions.host === "localhost") || (reqOptions.host === "127.0.0.1"))))
  { 
    return true;
  }
  else if (((reqOptions.hostname) && ((reqOptions.hostname === "[::1]") || (reqOptions.hostname === "[0:0:0:0:0:0:0:1]"))) || ((reqOptions.host) && ((reqOptions.host === "[::1]") || (reqOptions.host === "[0:0:0:0:0:0:0:1]"))))
  {
    return true;
  }
  else if (((reqOptions.hostname) && ((reqOptions.hostname === "::1") || (reqOptions.hostname === "0:0:0:0:0:0:0:1"))) || ((reqOptions.host) && ((reqOptions.host === "::1") || (reqOptions.host === "0:0:0:0:0:0:0:1"))))
  {
    return true;
  }
  return false;
}

function HttpsWrap()
{
  this.localApp = false;
}

HttpsWrap.prototype.setLocalApp = function(isLocalApp) {
  this.localApp = isLocalApp;
};

HttpsWrap.prototype.getLocalApp = function() {
  return this.localApp;
};

HttpsWrap.prototype.request = function(options, cb) {
  if (true == isLocalAccess(options))
  {
    if (false == this.localApp)
    {
      console.log("localhost urls cannot be accessed by remote applications");
      return;
    }
  }
  return https.request(options, cb);
};

HttpsWrap.prototype.get = function(options, cb) {
  if (true == isLocalAccess(options))
  {
    if (false == this.localApp)
    {
      console.log("localhost urls cannot be accessed by remote applications");
      return;
    }
  }
  return https.get(options, cb);
};

HttpsWrap.prototype.Agent = function(options) {
  if (true == isLocalAccess(options))
  {
    if (false == this.localApp)
    {
      console.log("localhost urls cannot be accessed by remote applications");
      return;
    }
  }
  return https.Agent(options);
};

HttpsWrap.prototype.globalAgent = function(options) {
  if (true == isLocalAccess(options))
  {
    if (false == this.localApp)
    {
      console.log("localhost urls cannot be accessed by remote applications");
      return;
    }
  }
  return https.globalAgent(options);
};

module.exports = HttpsWrap;
