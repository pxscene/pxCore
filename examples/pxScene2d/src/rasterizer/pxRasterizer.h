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


#ifndef  _H_PXRASTERIZER
#define _H_PXRASTERIZER

#include "px2d.h"

#include <stdint.h>

#include "pxMatrix4T.h"
#include "pxOffscreen.h"

#define USELONGCOVERAGE 1
#define EDGECLEANUP


//#define FRONT2BACK

class pxRasterizer
{
public:
  pxRasterizer();
  ~pxRasterizer();

  void init(pxBuffer* buffer);
  void term();

  void reset();

  void addEdge(double x1, double y1, double x2, double y2);

  void rasterize();
  //void rasterize2();

  pxFillMode fillMode() const;
  void setFillMode(const pxFillMode& m);

  pxColor color() const;
  void setColor(const pxColor& c);

  double alpha() const;
  void setAlpha(double alpha);

  // 1, 2, 4, 8, 16 supported
  // This will reset the rasterizer
  void setYOversample(int32_t i);

  // ??
  // This will reset the rasterizer
  // Valid values for this are 0, 1, 2, 4, 8, 16
  void setXResolution(int32_t i);

  pxRect clip();
  void setClip(const pxRect* r);

  pxBuffer* buffer()  const { return mBuffer; }
  pxBuffer* texture() const {return mTexture; }
  void setTexture(pxBuffer* texture);

  bool alphaTexture() const     { return mAlphaTexture; }
  void setAlphaTexture(bool f)  { mAlphaTexture = f;    }

  void setTextureCoordinates(pxVertex& e1, pxVertex& e2, pxVertex& e3, pxVertex& e4,
                             pxVertex& t1, pxVertex& t2, pxVertex& t3, pxVertex& t4);

  bool textureClamp() const    { return mTextureClamp; }
  void setTextureClamp(bool f) { mTextureClamp = f;    }

  bool textureClampColor() const    { return mTextureClampColor; }
  void setTextureClampColor(bool f) { mTextureClampColor = f;    }

	void matrix(pxMatrix4T<float>& m) const;
	void setMatrix(const pxMatrix4T<float>& m);

  void textureMatrix(pxMatrix4T<float>& m) const;
  void setTextureMatrix(const pxMatrix4T<float>& m);

  bool biLerp() const    { return mBiLerp; }
  void setBiLerp(bool f) { mBiLerp = f;    }

  bool overdraw() const    { return mOverdraw; }
  void setOverdraw(bool f) { mOverdraw = f;    }

  void clear();



private:

  void rasterizeComplex();

  void resetTextureEdges();
  void addTextureEdge(double x1, double y1, double x2, double y2,
                      double u1, double v1, double u2, double v2);

  inline void scanCoverage(pxPixel* scanline, int32_t x0, int32_t x1);
  inline pxPixel* getTextureSample(int32_t maxU, int32_t maxV, int32_t& curU, int32_t& curV);

  void calculateEffectiveAlpha();

  int32_t mYOversample;
  int32_t mXResolution;

  int32_t mFirst, mLast;
  int32_t mLeftExtent, mRightExtent;

  pxBuffer* mBuffer;

  //void* mStarts;
  //void* mEnds;

#ifndef EDGECLEANUP
  void* mEdgeArray;
  void * miStarts;
  void * miEnds;
  int32_t mEdgeCount;
#else
  void* mEdgeManager;
#endif

#ifdef USELONGCOVERAGE
  int8_t*  mCoverage;
#else
  uint8_t* mCoverage;
#endif

  pxFillMode mFillMode;
  pxPixel mColor;
  double mAlpha;

  bool mAlphaDirty;
  uint8_t mEffectiveAlpha;
  uint8_t mCoverage2Alpha[256];

  pxRect mClip;
  bool mClipValid;
  pxRect mClipInternal;  // mClip interescted with the bounds of the current buffer.
  bool mClipInternalCalculated;

  int32_t mCachedBufferHeight;
  int32_t mCachedBufferWidth;

  // mYOversample derived values
  uint32_t overSampleAdd;
  uint32_t overSampleAddMinusOne;
  uint32_t overSampleAdd4MinusOne;
  uint32_t overSampleAdd4;
  uint32_t overSampleFlush;
  uint32_t overSampleMask;
  //   uint32_t overSampleShift;

  pxBuffer* mTexture;

  bool mTextureClamp;
  bool mTextureClampColor;
  bool mBiLerp;
  bool mAlphaTexture;

  bool mOverdraw;

  uint8_t ltEdgeCover[16];  // static?  can be shared
  uint8_t rtEdgeCover[16];

  pxMatrix4T<float> mMatrix;
  pxMatrix4T<float> mTextureMatrix;

  int32_t mTextureOriginX, mTextureOriginY;

public:
//    double mExtentLeft, mExtentTop, mExtentRight, mExtentBottom;

}; // CLASS - pxRasterizer

#endif // _H_PXRASTERIZER

