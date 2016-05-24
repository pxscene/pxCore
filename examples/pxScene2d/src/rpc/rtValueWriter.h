#ifndef __RT_VALUE_WRITER_H__
#define __RT_VALUE_WRITER_H__

#include <memory>
#include "../rtValue.h"
#include "../rtError.h"
#include "rapidjson/document.h"

class rtValueWriter
{
public:
  static rtError write(rtValue const& from, rapidjson::Value& to, rapidjson::Document& parent);
};

#endif
