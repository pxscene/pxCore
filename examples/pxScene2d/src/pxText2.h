// pxCore CopyRight 2007-2015 John Robinson
// pxText2.h

#ifndef PX_TEXT2_H
#define PX_TEXT2_H

// TODO it would be nice to push this back into implemention
#include <ft2build.h>
#include FT_FREETYPE_H

#include "rtString.h"
#include "rtRefT.h"
#include "pxScene2d.h"
#include "pxText.h"


/*#### truncation - one of the following values
* NONE - no trunctaction
* TRUNCATE - text is truncated at the bottom of the text object.  The last word may be partially truncated. 
* TRUNCATE_AT_WORD_BOUNDARY - text is truncated at the bottom of the text object.  Truncation occurs at the word boundary.
* ELLIPSES - text is truncated at the bottom of the text object with ellipses applied.  The last word may be broken off by the ellipses.
* ELLIPSES_AT_WORD_BOUNDARY - text is truncated at the bottom of the text object with ellipses applied to the last possible full word.
###### Note: When truncation is applied at the word boundary, in a situation where the last line of text to be rendered contains a
*/
enum PX_TRUNCATION {
	NONE, 
	TRUNCATE,
	TRUNCATE_AT_WORD
};
enum PX_VERTICAL_ALIGN {
	V_TOP, 
	V_CENTER,
	V_BOTTOM
};
enum PX_HORIZONTAL_ALIGN {
	H_LEFT, 
	H_CENTER,
	H_RIGHT
};
#define ELLIPSIS_STR "\u2026"
#define ELLIPSIS_LEN (sizeof(ELLIPSIS_STR)-1)	
static const char isnew_line_chars[] = "\n\v\f\r";
static const char word_boundary_chars[] = " \t/:&,;.";
static const char space_chars[] = " \t";

/**********************************************************************
 * 
 * pxTextMetrics
 * 
 **********************************************************************/
class pxTextMetrics: public pxObject {

public:
	pxTextMetrics(pxScene2d* s): pxObject(s), mRefCount(0) {  }
	virtual ~pxTextMetrics() {}

	virtual unsigned long AddRef() 
	{
		return rtAtomicInc(&mRefCount);
	}

	virtual unsigned long Release() 
	{
		long l = rtAtomicDec(&mRefCount);
		if (l == 0) delete this;
			return l;
	}
	rtDeclareObject(pxTextMetrics, pxObject);
	rtReadOnlyProperty(height, height, float); 
	rtReadOnlyProperty(ascent, ascent, float);
	rtReadOnlyProperty(descent, descent, float);
  rtReadOnlyProperty(naturalLeading, naturalLeading, float);
  rtReadOnlyProperty(baseline, baseline, float);
 
	float height()             const { return mHeight; }
	rtError height(float& v)   const { v = mHeight; return RT_OK;   }
	rtError setHeight(float v)       { mHeight = v; return RT_OK;   }

	float ascent()             const { return mAscent; }
	rtError ascent(float& v)   const { v = mAscent; return RT_OK;   }
	rtError setAscent(float v)       { mAscent = v; return RT_OK;   } 

	float descent()             const { return mDescent; }
	rtError descent(float& v)   const { v = mDescent; return RT_OK;   }
	rtError setDescent(float v)       { mDescent = v; return RT_OK;   } 
 
 	float naturalLeading()             const { return mNaturalLeading; }
	rtError naturalLeading(float& v)   const { v = mNaturalLeading; return RT_OK;   }
	rtError setNaturalLeading(float v)       { mNaturalLeading = v; return RT_OK;   } 
  
 	float baseline()             const { return mBaseline; }
	rtError baseline(float& v)   const { v = mBaseline; return RT_OK;   }
	rtError setBaseline(float v)       { mBaseline = v; return RT_OK;   }   
   
  private:
    rtAtomic mRefCount;	
   	float mHeight;
    float mAscent;
    float mDescent;
    float mNaturalLeading;
    float mBaseline;
};

/**********************************************************************
 * 
 * pxCharPosition
 * 
 **********************************************************************/
class pxCharPosition: public pxObject {

public:
	pxCharPosition(pxScene2d* s): pxObject(s), mRefCount(0) {  }
	virtual ~pxCharPosition() {}

	virtual unsigned long AddRef() 
	{
		return rtAtomicInc(&mRefCount);
	}

	virtual unsigned long Release() 
	{
		long l = rtAtomicDec(&mRefCount);
		if (l == 0) delete this;
			return l;
	}
  
	rtDeclareObject(pxCharPosition, pxObject);
	rtReadOnlyProperty(x, x, float);
  rtReadOnlyProperty(y, y, float);

	float x()             const { return mX; }
	rtError x(float& v)   const { v = mX; return RT_OK;   }
	rtError setX(float v)       { mX = v; return RT_OK;   }

	float y()             const { return mY; }
	rtError y(float& v)   const { v = mY; return RT_OK;   }
	rtError setY(float v)       { mY = v; return RT_OK;   }

  void clear() {
    mX = 0;
    mY = 0;
  }
      
  private:
    rtAtomic mRefCount;	
    float mX;
    float mY;
    
};

/**********************************************************************
 * 
 * pxTextBounds
 * 
 **********************************************************************/
class pxTextBounds: public pxObject {

public:
	pxTextBounds(pxScene2d* s): pxObject(s), mRefCount(0) { clear(); }
	virtual ~pxTextBounds() {}

	virtual unsigned long AddRef() 
	{
		return rtAtomicInc(&mRefCount);
	}

	virtual unsigned long Release() 
	{
		long l = rtAtomicDec(&mRefCount);
		if (l == 0) delete this;
			return l;
	}
  
	rtDeclareObject(pxTextBounds, pxObject);
	rtReadOnlyProperty(x1, x1, float);
  rtReadOnlyProperty(y1, y1, float);
  rtReadOnlyProperty(x2, x2, float);
  rtReadOnlyProperty(y2, y2, float);

	float x1()             const { return mX1; }
	rtError x1(float& v)   const { v = mX1; return RT_OK;   }
	rtError setX1(float v)       { mX1 = v; return RT_OK;   }

	float y1()             const { return mY1; }
	rtError y1(float& v)   const { v = mY1; return RT_OK;   }
	rtError setY1(float v)       { mY1 = v; return RT_OK;   }

	float x2()             const { return mX2; }
	rtError x2(float& v)   const { v = mX2; return RT_OK;   }
	rtError setX2(float v)       { mX2 = v; return RT_OK;   }

	float y2()             const { return mY2; }
	rtError y2(float& v)   const { v = mY2; return RT_OK;   }
	rtError setY2(float v)       { mY2 = v; return RT_OK;   }
      
  void clear() {
    mX1 = 0;
    mY1 = 0;
    mX2 = 0;
    mY2 = 0;
  }    
  private:
    rtAtomic mRefCount;	
    float mX1;
    float mY1;
    float mX2;
    float mY2;  
    
};
/**********************************************************************
 * 
 * pxTextMeasurements
 * 
 **********************************************************************/
class pxTextMeasurements: public pxObject {

public:
	pxTextMeasurements(pxScene2d* s): pxObject(s), mRefCount(0) { 
    mBounds = new pxTextBounds(s);
    mFirstChar = new pxCharPosition(s);
    mLastChar = new pxCharPosition(s);
  }
	virtual ~pxTextMeasurements() {}

	virtual unsigned long AddRef() 
	{
		return rtAtomicInc(&mRefCount);
	}

	virtual unsigned long Release() 
	{
		long l = rtAtomicDec(&mRefCount);
		if (l == 0) delete this;
			return l;
	}
	rtDeclareObject(pxTextMeasurements, pxObject);
  rtReadOnlyProperty(bounds, bounds, rtObjectRef);
  rtReadOnlyProperty(firstChar, firstChar, rtObjectRef);
  rtReadOnlyProperty(lastChar, lastChar, rtObjectRef);
  
  rtError bounds(rtObjectRef& v) const
  {
    v = mBounds;
    return RT_OK;
  }
  rtError firstChar(rtObjectRef& v) const
  {
    v = mFirstChar;
    return RT_OK;
  }
  rtError lastChar(rtObjectRef& v) const
  {
    v = mLastChar;
    return RT_OK;
  } 
  
  rtRefT<pxTextBounds> getBounds()      { return mBounds;}
  rtRefT<pxCharPosition> getFirstChar() { return mFirstChar; }
  rtRefT<pxCharPosition> getLastChar()  { return mLastChar; }
  
  void clear() {
    mBounds->clear();
    mFirstChar->clear();
    mLastChar->clear();
  }    
      
  private:
    rtAtomic mRefCount;	
    
    rtRefT<pxTextBounds> mBounds;
    rtRefT<pxCharPosition> mFirstChar;
    rtRefT<pxCharPosition> mLastChar;
    
};

/**********************************************************************
 * 
 * pxTex2
 * 
 **********************************************************************/
class pxText2: public pxText {
public:
  rtDeclareObject(pxText2, pxText);

  pxText2(pxScene2d* s);
  ~pxText2();
  
  rtProperty(wordWrap, wordWrap, setWordWrap, bool);
  rtProperty(ellipsis, ellipsis, setEllipsis, bool);
  rtProperty(xStartPos, xStartPos, setXStartPos, float); 
  rtProperty(xStopPos, xStopPos, setXStopPos, float);
  rtProperty(truncation, truncation, setTruncation, uint32_t);
  rtProperty(verticalAlign, verticalAlign, setVerticalAlign, uint32_t);
  rtProperty(horizontalAlign, horizontalAlign, setHorizontalAlign, uint32_t);
  rtProperty(leading, leading, setLeading, float); 	
  
  bool wordWrap()            const { return mWordWrap;}
  rtError wordWrap(bool& v)  const { v = mWordWrap; return RT_OK;  }
  rtError setWordWrap(bool v) { mWordWrap = v; recalc(); return RT_OK; }
  
  bool ellipsis()            const { return mEllipsis;}
  rtError ellipsis(bool& v)  const { v = mEllipsis; return RT_OK;  }
  rtError setEllipsis(bool v) { mEllipsis = v; recalc(); return RT_OK; }
  
  float xStartPos()             const { return mXStartPos; }
  rtError xStartPos(float& v)   const { v = mXStartPos; return RT_OK;   }
  rtError setXStartPos(float v) { mXStartPos = v; recalc(); return RT_OK; }
  
  float xStopPos()             const { return mXStopPos; }
  rtError xStopPos(float& v)   const { v = mXStopPos; return RT_OK;   }
  rtError setXStopPos(float v) { mXStopPos = v; recalc(); return RT_OK; }
    
  uint32_t truncation()             const { return mTruncation; }
  rtError truncation(uint32_t& v)   const { v = mTruncation; return RT_OK;   }
  rtError setTruncation(uint32_t v)       { mTruncation = v; recalc(); return RT_OK;   }

  uint8_t verticalAlign()             const { return mVerticalAlign; }
  rtError verticalAlign(uint32_t& v)   const { v = mVerticalAlign; return RT_OK;   }
  rtError setVerticalAlign(uint32_t v)       { mVerticalAlign = v; recalc(); return RT_OK;   }
  
  uint8_t horizontalAlign()             const { return mHorizontalAlign; }
  rtError horizontalAlign(uint32_t& v)   const { v = mHorizontalAlign; return RT_OK;   }
  rtError setHorizontalAlign(uint32_t v)       { mHorizontalAlign = v; recalc(); return RT_OK;   }
  
  float leading()             const { return mLeading; }
  rtError leading(float& v)   const { v = mLeading; return RT_OK;   }
  rtError setLeading(float v)       { mLeading = v; recalc(); return RT_OK;   }  
  
  virtual rtError setText(const char* s); 
  virtual rtError setPixelSize(uint32_t v);
  void renderText(bool render);
  virtual void fontLoaded();
  void determineMeasurementBounds();
  virtual void draw();
  virtual void onInit();

 
  rtMethodNoArgAndReturn("getFontMetrics", getFontMetrics, rtObjectRef);
  rtError getFontMetrics(rtObjectRef& o);
  rtMethodNoArgAndReturn("measureText", measureText, rtObjectRef);
  rtError measureText(rtObjectRef& o); 

  virtual rtError Set(const char* name, const rtValue* value)
  {
	  //printf("pxText2 Set for %s\n", name );

    mDirty = mDirty || (!strcmp(name,"wordWrap") ||
              !strcmp(name,"ellipsis") ||
              !strcmp(name,"xStartPos") ||
              !strcmp(name,"xStopPos") ||
              !strcmp(name,"truncation") ||
              !strcmp(name,"verticalAlign")||
              !strcmp(name,"horizontalAlign") ||
              !strcmp(name,"leading"));

    rtError e = pxText::Set(name, value);

    return e;
  }


 protected:
 
	uint32_t mTruncation;  
	float mXStartPos;
	float mXStopPos;
	uint32_t mVerticalAlign;
	uint32_t mHorizontalAlign;
	float mLeading;
	bool mWordWrap;
	bool mEllipsis;
  
  bool mFontLoaded;
  bool mInitialized;
  
  rtRefT<pxTextMeasurements> measurements;
  uint32_t lineNumber;
  uint32_t lastLineNumber;
  
  bool isNewline( char ch );
  bool isWordBoundary( char ch );
  bool isSpaceChar( char ch );  
  
  void renderTextRowWithTruncation(rtString & accString, float lineWidth, float tempY, float sx, float sy, uint32_t pixelSize,float* color, bool render);
  void renderTextNoWordWrap(float sx, float sy, float tempX, bool render);
  void renderTextWithWordWrap(const char *text, float sx, float sy, float tempX, uint32_t pixelSize, float* color, bool render);
  void measureTextWithWrapOrNewLine(const char *text, float sx, float sy, float tempX, float &tempY, uint32_t size, float* color, uint32_t &lineNumber, bool render);
  void renderOneLine(uint32_t lineNumber, const char * tempStr, float tempX, float tempY, float sx, float sy,  uint32_t size, float* color, float lineWidth, bool render );
  
  void recalc();
  void clearMeasurements();
  void setMeasurementBounds(float xPos, float width, float yPos, float height);
  void setLastLineMeasurements(float xPos, float y);
};

#endif
