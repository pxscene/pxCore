/*

 pxCore Copyright 2005-2017 John Robinson

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

#ifndef _RT_PERMISSIONS
#define _RT_PERMISSIONS

#include "rtObject.h"
#include "rtRef.h"
#include "rtAtomic.h"

#include <map>
#include <string>
#include <utility>

#define rtPermissionsCheck(permissionsRef, id, type)\
  if (permissionsRef != NULL)\
  {\
    bool o;\
    permissionsRef->allows(id, type, o);\
    if (!o)\
    {\
      rtLogError("'%s' is not allowed", id);\
      return RT_ERROR_NOT_ALLOWED;\
    }\
  }

class rtPermissions;
typedef rtRef<rtPermissions> rtPermissionsRef;

class rtPermissions
{
public:
  rtPermissions(const char* origin = NULL);
  virtual ~rtPermissions();

  virtual unsigned long AddRef(){ return rtAtomicInc(&mRef);}

  virtual unsigned long Release()
  {
    unsigned long l = rtAtomicDec(&mRef);
    if (l == 0)
      delete this;
    return l;
  }

  enum Type
  {
    DEFAULT = 0,
    SERVICE,
    FEATURE,
    WAYLAND
  };

  rtError set(const rtObjectRef& permissionsObject);
  rtError setParent(const rtPermissionsRef& parent);
  rtError allows(const char* s, rtPermissions::Type type, bool& o) const;

protected:
  // Wildcard stuff
  typedef std::pair<std::string, Type> wildcard_t;
  typedef std::map<wildcard_t, bool> permissionsMap_t;
  typedef std::map<wildcard_t, std::string> assignMap_t;
  typedef std::map<std::string, permissionsMap_t> roleMap_t;
  // Extends std::map::find by supporting wildcard_t as map keys.
  // Key with the highest length w/o wildcards (*) is preferred
  template<typename Map> typename Map::const_iterator
  static findWildcard(Map const& map, typename Map::key_type const& key);

  // Parsing
  static permissionsMap_t permissionsJsonToMap(const void* jsonValue);
  static permissionsMap_t permissionsObjectToMap(const rtObjectRef& permissionsObject);

  // Bootstrap
  static const char* DEFAULT_CONFIG_FILE;
  static const int CONFIG_BUFFER_SIZE;
  static const char* CONFIG_ENV_NAME;
  static rtError loadConfig();
  static assignMap_t mAssignMap;
  static roleMap_t mRolesMap;
  static std::string mConfigPath;

  permissionsMap_t mPermissionsMap;
  rtPermissionsRef mParent;
  rtAtomic mRef;

  friend class rtPermissionsTest;
};

#endif
