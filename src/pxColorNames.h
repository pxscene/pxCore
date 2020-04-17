/*
  pxCore Copyright 2005-2019 John Robinson

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  Author: Hugh Fitzpatrick
*/

// pxColorNames.h

#ifndef PX_COLORNAMES_H
#define PX_COLORNAMES_H

#include "rtError.h"
#include "rtString.h"

rtError web2rgb(rtString &input, uint8_t &r, uint8_t &g, uint8_t &b, uint8_t &a);

rtError web2rgba(rtString &input, uint32_t &rgba);
rtError web2argb(rtString &input, uint32_t &argb);

#endif // PX_COLORNAMES_H
