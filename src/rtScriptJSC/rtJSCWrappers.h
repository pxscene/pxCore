/*

 pxCore Copyright 2005-2018 John Robinson

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

#ifndef RTJSCWRAPPERS_H
#define RTJSCWRAPPERS_H

#include <JavaScriptCore/JavaScript.h>

#include <unordered_set>

#include "rtJSCMisc.h"
#include "rtCore.h"
#include "rtObject.h"
#include "rtValue.h"
#include "rtScriptJSCPrivate.h"

namespace RtJSC {

rtError jsToRt(JSContextRef context, JSValueRef value, rtValue &result, JSValueRef *exception);
JSValueRef rtToJs(JSContextRef context, const rtValue &rtval);

class JSObjectWrapper: public RefCounted<rtIObject>, public rtJSCWrapperBase
{
  bool m_isArray { false };
public:
  JSObjectWrapper(JSContextRef context, JSObjectRef object, bool isArray);
  ~JSObjectWrapper();

  rtMethodMap* getMap() const override { return nullptr; }
  rtError Get(const char* name, rtValue* value) const override;
  rtError Get(uint32_t i, rtValue* value) const override;
  rtError Set(const char* name, const rtValue* value) override;
  rtError Set(uint32_t i, const rtValue* value) override;
};

class JSFunctionWrapper: public RefCounted<rtIFunction>, public rtJSCWrapperBase
{
  size_t hash() override { return mHash; }
  void setHash(size_t hash) override { UNUSED_PARAM(hash); }
  rtError Send(int numArgs, const rtValue* args, rtValue* result) override;
  rtJSCWeak m_thisObj;
  size_t mHash;
public:
  JSFunctionWrapper(JSContextRef context, JSObjectRef thisObj, JSObjectRef funcObj);
  JSFunctionWrapper(JSContextRef context, JSObjectRef funcObj);
  ~JSFunctionWrapper();
};

}  // RtJSC

#endif /* RTJSCWRAPPERS_H */
