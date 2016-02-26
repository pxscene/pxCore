// pxCore CopyRight 2007-2015 John Robinson
// pxScene2d.h

#ifndef PX_SCENE2D_H
#define PX_SCENE2D_H

#include <stdio.h>

#include <vector>
#include <list>
using namespace std;

#ifndef finline
#ifdef WIN32
#define finline __forceinline
#else
#define finline __attribute__((always_inline))
#endif
#endif

#include "rtRefT.h"
#include "rtString.h"

// TODO rtDefs vs rtCore.h
#include "rtDefs.h"
#include "rtCore.h"
#include "rtError.h"
#include "rtValue.h"
#include "rtObject.h"
#include "rtObjectMacros.h"
#include "rtPromise.h"
#include "rtThreadQueue.h"
#include "pxResource.h"

#include "pxCore.h"
#include "pxIView.h"

#include "pxMatrix4T.h"
#include "pxInterpolators.h"
#include "pxTexture.h"
//#include "pxTextureCacheObject.h"
#include "pxContextFramebuffer.h"

#include "pxArchive.h"

#include "testView.h"

//Uncomment to enable display of pointer by pxScene
//#define USE_SCENE_POINTER

// TODO Move this to pxEventLoop
extern rtThreadQueue gUIThreadQueue;

// TODO Finish
//#include "pxTransform.h"
#include "pxConstants.h"

// Constants
static pxConstants CONSTANTS;


#if 0
typedef rtError (*objectFactory)(void* context, const char* t, rtObjectRef& o);
void registerObjectFactory(objectFactory f, void* context);
rtError createObject2(const char* t, rtObjectRef& o);
#endif

typedef void (*pxAnimationEnded)(void* ctx);


struct pxAnimationTarget {
  char* prop;
  float to;
};

struct animation {
  bool cancelled;
  rtString prop;
  float from;
  float to;
  bool flip;
  double start;
  double duration;
  pxConstantsAnimation::animationOptions at;
  pxInterp interp;
  int32_t count;
  float actualCount;
  bool reversing;
  rtFunctionRef ended;
  rtObjectRef promise;
};

struct pxPoint2f {
  pxPoint2f() {}
  pxPoint2f(float _x, float _y) { x = _x; y = _y; } 
  float x, y;
};


class pxFileDownloadRequest;

class pxScene2d;

class pxObjectImpl
{
protected:
  vector<animation> mAnimations;  
};

class pxObject: public rtObject, private pxObjectImpl
{
public:
  rtDeclareObject(pxObject, rtObject);
  rtReadOnlyProperty(_pxObject, _pxObject, voidPtr);
  rtProperty(parent, parent, setParent, rtObjectRef);
  rtProperty(x, x, setX, float); 
  rtProperty(y, y, setY, float);
  rtProperty(w, w, setW, float);
  rtProperty(h, h, setH, float);
  rtProperty(cx, cx, setCX, float);
  rtProperty(cy, cy, setCY, float);
  rtProperty(sx, sx, setSX, float);
  rtProperty(sy, sy, setSY, float);
  rtProperty(a, a, setA, float);
  rtProperty(r, r, setR, float);
#ifdef ANIMATION_ROTATE_XYZ
  rtProperty(rx, rx, setRX, float);
  rtProperty(ry, ry, setRY, float);
  rtProperty(rz, rz, setRZ, float);
#endif // ANIMATION_ROTATE_XYZ
  rtProperty(id, id, setId, rtString);
  rtProperty(interactive, interactive, setInteractive, bool);
  rtProperty(painting, painting, setPainting, bool);
  rtProperty(clip, clip, setClip, bool);
  rtProperty(mask, mask, setMask, bool);
  rtProperty(draw, drawEnabled, setDrawEnabled, bool);
  rtProperty(hitTest, hitTest, setHitTest, bool);
  rtProperty(focus, focus, setFocus, bool); 
  rtReadOnlyProperty(ready, ready, rtObjectRef);

  rtReadOnlyProperty(numChildren, numChildren, int32_t);
  rtMethod1ArgAndReturn("getChild", getChild, int32_t, rtObjectRef);
  rtReadOnlyProperty(children, children, rtObjectRef);

  rtMethodNoArgAndNoReturn("remove", remove);
  rtMethodNoArgAndNoReturn("removeAll", removeAll);
  rtMethodNoArgAndNoReturn("moveToFront", moveToFront);

  rtMethod5ArgAndReturn("animateTo", animateToP2, rtObjectRef, double,
                        uint32_t, uint32_t, int32_t, rtObjectRef);

  rtMethod2ArgAndNoReturn("on", addListener, rtString, rtFunctionRef);
  rtMethod2ArgAndNoReturn("delListener", delListener, rtString, rtFunctionRef);
 // rtProperty(onReady, onReady, setOnReady, rtFunctionRef);

//  rtReadOnlyProperty(emit, emit, rtFunctionRef);
  rtMethod1ArgAndReturn("getObjectById",getObjectById,rtString,rtObjectRef);

  rtProperty(m11,m11,setM11,float);
  rtProperty(m12,m12,setM12,float);
  rtProperty(m13,m13,setM13,float);
  rtProperty(m14,m14,setM14,float);
  rtProperty(m21,m21,setM21,float);
  rtProperty(m22,m22,setM22,float);
  rtProperty(m23,m23,setM23,float);
  rtProperty(m24,m24,setM24,float);
  rtProperty(m31,m31,setM31,float);
  rtProperty(m32,m32,setM32,float);
  rtProperty(m33,m33,setM33,float);
  rtProperty(m34,m34,setM34,float);
  rtProperty(m41,m41,setM41,float);
  rtProperty(m42,m42,setM42,float);
  rtProperty(m43,m43,setM43,float);
  rtProperty(m44,m44,setM44,float);
  rtProperty(useMatrix,useMatrix,setUseMatrix,bool);

  pxObject(pxScene2d* scene): rtObject(), mcx(0), mcy(0), mx(0), my(0), ma(1.0), mr(0),
#ifdef ANIMATION_ROTATE_XYZ  
    mrx(0), mry(0), mrz(1.0), 
#endif //ANIMATION_ROTATE_XYZ
    msx(1), msy(1), mw(0), mh(0),
    mInteractive(true),
    mSnapshotRef(), mPainting(true), mClip(false), mMask(false), mDraw(true), mHitTest(true), mReady(), 
    mFocus(false),mClipSnapshotRef(),mCancelInSet(true),mUseMatrix(false), mRepaint(true)
      , mRepaintCount(0) //TODO - remove mRepaintCount as it's only needed on certain platforms
#ifdef PX_DIRTY_RECTANGLES
    , mIsDirty(false), mLastRenderMatrix(), mScreenCoordinates()
    #endif //PX_DIRTY_RECTANGLES
  {
    mScene = scene;
    mReady = new rtPromise;
    mEmit = new rtEmit;
  }

  virtual ~pxObject() { /*printf("pxObject destroyed\n");*/ rtValue nullValue; mReady.send("reject",nullValue); deleteSnapshot(mSnapshotRef); deleteSnapshot(mClipSnapshotRef);}

  // TODO missing conversions in rtValue between uint32_t and int32_t
  uint32_t numChildren() const { return mChildren.size(); }
  rtError numChildren(int32_t& v) const 
  {
    v = mChildren.size();
    return RT_OK;
  }

  virtual rtError Set(const char* name, const rtValue* value);

  rtError getChild(int32_t i, rtObjectRef& r) const 
  {
    r = mChildren[i];
    return RT_OK;
  }

  rtError children(rtObjectRef& v) const;

  // TODO clean this up
  void setParent(rtRefT<pxObject>& parent);
  pxObject* parent() const
  {
    return mParent;
  }

  rtError parent(rtObjectRef& v) const 
  {
    v = mParent.getPtr();
    return RT_OK;
  }

  rtError setParent(rtObjectRef parent) 
  {
    void* p = parent.get<voidPtr>("_pxObject");
    if (p) {
      rtRefT<pxObject> p2 = (pxObject*)p;
      setParent(p2);
    }
    return RT_OK;
  }

  rtError remove();
  rtError removeAll();
  
  rtString id() { return mId; }
  rtError id(rtString& v) const { v = mId; return RT_OK; }
  rtError setId(const rtString& v) { mId = v; return RT_OK; }

  rtError interactive(bool& v) const { v = mInteractive; return RT_OK; }
  rtError setInteractive(bool v) { mInteractive = v; return RT_OK; }

  float x()             const { return mx; }
  rtError x(float& v)   const { v = mx; return RT_OK;   }
  rtError setX(float v)       { cancelAnimation("x"); mx = v; return RT_OK;   }
  float y()             const { return my; }
  rtError y(float& v)   const { v = my; return RT_OK;   }
  rtError setY(float v)       { cancelAnimation("y"); my = v; return RT_OK;   }
  float w()             const { return mw; }
  rtError w(float& v)   const { v = mw; return RT_OK;   }
  virtual rtError setW(float v)       { cancelAnimation("w"); createNewPromise();mw = v; return RT_OK;   }
  float h()             const { return mh; }
  rtError h(float& v)   const { v = mh; return RT_OK;   }
  virtual rtError setH(float v)       { cancelAnimation("h"); createNewPromise();mh = v; return RT_OK;   }
  float cx()            const { return mcx;}
  rtError cx(float& v)  const { v = mcx; return RT_OK;  }
  rtError setCX(float v)      { cancelAnimation("cx"); createNewPromise();mcx = v; return RT_OK;  }
  float cy()            const { return mcy;}
  rtError cy(float& v)  const { v = mcy; return RT_OK;  }
  rtError setCY(float v)      { cancelAnimation("cy"); createNewPromise();mcy = v; return RT_OK;  }
  float sx()            const { return msx;}
  rtError sx(float& v)  const { v = msx; return RT_OK;  }
  rtError setSX(float v)      { cancelAnimation("sx"); createNewPromise();msx = v; return RT_OK;  }
  float sy()            const { return msy;}
  rtError sy(float& v)  const { v = msx; return RT_OK;  } 
  rtError setSY(float v)      { cancelAnimation("sy");createNewPromise(); msy = v; return RT_OK;  }
  float a()             const { return ma; }
  rtError a(float& v)   const { v = ma; return RT_OK;   }
  rtError setA(float v)       { cancelAnimation("a"); ma = v; return RT_OK;   }
  float r()             const { return mr; }
  rtError r(float& v)   const { v = mr; return RT_OK;   }
  rtError setR(float v)       { cancelAnimation("r"); createNewPromise();mr = v; return RT_OK;   }
#ifdef ANIMATION_ROTATE_XYZ
  float rx()            const { return mrx;}
  rtError rx(float& v)  const { v = mrx; return RT_OK;  }
  rtError setRX(float v)      { cancelAnimation("rx"); createNewPromise(); mrx = v; return RT_OK;  }
  float ry()            const { return mry;}
  rtError ry(float& v)  const { v = mry; return RT_OK;  }
  rtError setRY(float v)      { cancelAnimation("ry"); createNewPromise();mry = v; return RT_OK;  }
  float rz()            const { return mrz;}
  rtError rz(float& v)  const { v = mrz; return RT_OK;  }
  rtError setRZ(float v)      { cancelAnimation("rz"); createNewPromise();mrz = v; return RT_OK;  }
#endif // ANIMATION_ROTATE_XYZ
  bool painting()            const { return mPainting;}
  rtError painting(bool& v)  const { v = mPainting; return RT_OK;  }
  rtError setPainting(bool v)
  { 
      mPainting = v; 
      if (!mPainting)
      {
        //rtLogInfo("in setPainting and calling createSnapshot mw=%f mh=%f\n", mw, mh);
        mSnapshotRef = createSnapshot(mSnapshotRef);
      }
      else
      {
          deleteSnapshot(mSnapshotRef);
      }
      return RT_OK;
  }

  bool clip()            const { return mClip;}
  rtError clip(bool& v)  const { v = mClip; return RT_OK;  }
  virtual rtError setClip(bool v) { mClip = v; return RT_OK; }

  bool mask()            const { return mMask;}
  rtError mask(bool& v)  const { v = mMask; return RT_OK;  }
  rtError setMask(bool v) { mMask = v; return RT_OK; }

  bool drawEnabled()            const { return mDraw;}
  rtError drawEnabled(bool& v)  const { v = mDraw; return RT_OK;  }
  rtError setDrawEnabled(bool v) { mDraw = v; return RT_OK; }

  bool hitTest()            const { return mHitTest;}
  rtError hitTest(bool& v)  const { v = mHitTest; return RT_OK;  }
  rtError setHitTest(bool v) { mHitTest = v; return RT_OK; }

  bool focus()            const { return mFocus;}
  rtError focus(bool& v)  const { v = mFocus; return RT_OK;  }
  rtError setFocus(bool v);
  
  rtError ready(rtObjectRef& v) const
  {
    v = mReady;
    return RT_OK;
  }

  rtError moveToFront();
  void moveToBack();
  void moveForward();
  void moveBackward();

  void drawInternal(bool maskPass=false);
  virtual void draw() {}
  virtual void sendPromise();
  virtual void createNewPromise();

  bool hitTestInternal(pxMatrix4f m, pxPoint2f& pt, rtRefT<pxObject>& hit, pxPoint2f& hitPt);
  virtual bool hitTest(pxPoint2f& pt);

  void setFocusInternal(bool focus) { mFocus = focus; }
  
  rtError animateTo(const char* prop, double to, double duration,
                     uint32_t interp, uint32_t animationType, 
                     int32_t count, rtObjectRef promise);

  rtError animateToP2(rtObjectRef props, double duration, 
                      uint32_t interp, uint32_t animationType,
                      int32_t count, rtObjectRef& promise);

  void animateToInternal(const char* prop, double to, double duration,
                 pxInterp interp, pxConstantsAnimation::animationOptions,
                 int32_t count, rtObjectRef promise);

  void cancelAnimation(const char* prop, bool fastforward = false, bool rewind = false, bool resolve = false);

  rtError addListener(rtString eventName, const rtFunctionRef& f)
  {
    return mEmit->addListener(eventName, f);
  }

  rtError delListener(rtString  eventName, const rtFunctionRef& f)
  {
    return mEmit->delListener(eventName, f);
  }

  //rtError onReady(rtFunctionRef& /*f*/) const
  //{
    //rtLogError("onReady get not implemented\n");
    //return RT_OK;
  //}

  // TODO why does this have to be const
  //rtError setOnReady(const rtFunctionRef& f)
  //{
    //mEmit->setListener("onReady", f);
    //return RT_OK;
  //}

  virtual void update(double t);

  // non-destructive applies transform on top of of provided matrix
  virtual void applyMatrix(pxMatrix4f& m)
  {
#if 0
    rtRefT<pxTransform> t = new pxTransform;
    rtObjectRef i = new rtMapObject();
    i.set("x",0);
    i.set("y",0);
    i.set("cx",0);
    i.set("cy",0);
    i.set("sx",1);
    i.set("sy",1);
    i.set("r",0);
#ifdef ANIMATION_ROTATE_XYZ    
    i.set("rx",0);
    i.set("ry",0);
    i.set("rz",1);
#endif //ANIMATION_ROTATE_XYZ
    printf("before initTransform\n");
    t->initTransform(i, 
      "x cx + y cy + translateXY "
#ifdef ANIMATION_ROTATE_XYZ
      "r rx ry rz rotateInDegreesXYZ "
#else
      "r rotateInDegrees rotateInDegreesXYZ "
#endif //ANIMATION_ROTATE_XYZ
      "sx sy scaleXY "
      "cx -1 * cy -1 * translateXY "
      );
    printf("after initTransform\n");
    pxTransformData* d = t->newData();
    if (d)
    {
#if 1
      pxMatrix4f m;
      
      d->set("x",100);
      d->set("y",100);
      
      float v;
      d->get("x", v);
      d->get("cx", v);
      
      printf("Before applyMatrix\n");    
      d->applyMatrix(m);
      printf("After applyMatrix\n");    
     
#endif 
      t->deleteData(d);
      printf("After deleteData\n");
    }
    else
      rtLogError("Could not allocate pxTransformData");
#endif
    if (!mUseMatrix)
    {
#if 1
      // translate based on xy rotate/scale based on cx, cy
      m.translate(mx+mcx, my+mcy);
      if (mr) {
        m.rotateInDegrees(mr
#ifdef ANIMATION_ROTATE_XYZ
        , mrx, mry, mrz
#endif // ANIMATION_ROTATE_XYZ        
        );
      }
      if (msx != 1.0 || msy != 1.0) m.scale(msx, msy);  
      m.translate(-mcx, -mcy);    
#else
      // translate/rotate/scale based on cx, cy
      m.translate(mx, my);
      if (mr) {
        m.rotateInDegrees(mr
#ifdef ANIMATION_ROTATE_XYZ
        , mrx, mry, mrz
#endif // ANIMATION_ROTATE_XYZ        
        );
      }
      if (msx != 1.0 || msy != 1.0) m.scale(msx, msy);
      m.translate(-mcx, -mcy);
#endif
    }
    else
    {
      m.multiply(mMatrix);
    }
  }

  static void getMatrixFromObjectToScene(pxObject* o, pxMatrix4f& m) {
#if 1
    m.identity();
    
    while(o)
    {
      pxMatrix4f m2;
#if 0
      m2.translate(j->mx+j->mcx, j->my+j->mcy);
      if (j->mr) {
        m2.rotateInDegrees(j->mr
#ifdef ANIMATION_ROTATE_XYZ        
        , j->mrx, j->mry, j->mrz
#endif //ANIMATION_ROTATE_XYZ        
        );
      }
      if (j->msx != 1.0 || j->msy != 1.0) m2.scale(j->msx, j->msy);  
      m2.translate(-j->mcx, -j->mcy);
#else
      o->applyMatrix(m2);
#endif
      // TODO adding a different operator here would eliminate the copy
      m2.multiply(m);
      m = m2;
      o = o->mParent;
    }
#else
    getMatrixFromSceneToObject(o, m);
    m.invert();
#endif
  }
  
  static void getMatrixFromSceneToObject(pxObject* o, pxMatrix4f& m) {
#if 0
    m.identity();
    
    vector<rtRefT<pxObject> > v;
    rtRefT<pxObject> t = o;
    
    while(t) {
      v.push_back(t);
      t = t->mParent;
    }
    
    for(vector<rtRefT<pxObject> >::reverse_iterator it = v.rbegin(); it != v.rend(); ++it) 
    {
      rtRefT<pxObject>& j = *it;;
      pxMatrix4f m2;
      m2.translate(j->mx+j->mcx, j->my+j->mcy);
      if (j->mr) {
        m2.rotateInDegrees(j->mr
#ifdef ANIMATION_ROTATE_XYZ        
        , j->mrx, j->mry, j->mrz
#endif //ANIMATION_ROTATE_XYZ      
        );
      }
      
      if (j->msx != 1.0 || j->msy != 1.0) m2.scale(j->msx, j->msy);  
      m2.translate(-j->mcx, -j->mcy);
      m2.invert();
      m2.multiply(m);
      m = m2;
    }
#else
    getMatrixFromObjectToScene(o, m);
    m.invert();
#endif
  }
  
  static void getMatrixFromObjectToObject(pxObject* from, pxObject* to, pxMatrix4f& m) {
    pxMatrix4f t;
    getMatrixFromObjectToScene(from, t);
    getMatrixFromSceneToObject(to, m);
    
    m.multiply(t);
  }
  
  static void transformPointFromObjectToScene(pxObject* o, const pxVector4f& from, pxVector4f& to) {
    pxMatrix4f m;
    getMatrixFromObjectToScene(o, m);
    to = m.multiply(from);
  }
  
  static void transformPointFromSceneToObject(pxObject* o, const pxVector4f& from, pxVector4f& to) {
    pxMatrix4f m;
    getMatrixFromSceneToObject(o, m);
    to = m.multiply(from);
  }
  
  static void transformPointFromObjectToObject(pxObject* fromObject, pxObject* toObject, pxVector4f& from, pxVector4f& to) {
    pxMatrix4f m;
    getMatrixFromObjectToObject(fromObject, toObject, m);
    to = m.multiply(from);
  }

  rtError emit(rtFunctionRef& v) const { v = mEmit; return RT_OK; }
  
  static pxObject* getObjectById(const char* id, pxObject* from)
  {
    // TODO fix rtString empty check
    if (from->mId.cString() && !strcmp(id, from->mId.cString()))
      return from;
    
    for(vector<rtRefT<pxObject> >::iterator it = from->mChildren.begin(); it != from->mChildren.end(); ++it)
    {
      pxObject* o = getObjectById(id, (*it).getPtr());
      if (o)
        return o;
    }
    
    return NULL;
  }

  rtError getObjectById(const char* id, rtObjectRef& o)
  {
    o = getObjectById(id, this);
    return RT_OK;
  }

  virtual bool onTextureReady();
  // !CLF: To Do: These names are terrible... find better ones!
  // These to functions are not exposed to javascript; they are for internal
  // determination of w/h for the pxObject. For instance, pxImage could be
  // from the texture or the pxImage values themselves.
  virtual float getOnscreenWidth() {  return mw; }
  virtual float getOnscreenHeight() { return mh;  }

  rtError m11(float& v) const { v = mMatrix.constData(0); return RT_OK; }
  rtError m12(float& v) const { v = mMatrix.constData(1); return RT_OK; }
  rtError m13(float& v) const { v = mMatrix.constData(2); return RT_OK; }
  rtError m14(float& v) const { v = mMatrix.constData(3); return RT_OK; }
  rtError m21(float& v) const { v = mMatrix.constData(4); return RT_OK; }
  rtError m22(float& v) const { v = mMatrix.constData(5); return RT_OK; }
  rtError m23(float& v) const { v = mMatrix.constData(6); return RT_OK; }
  rtError m24(float& v) const { v = mMatrix.constData(7); return RT_OK; }
  rtError m31(float& v) const { v = mMatrix.constData(8); return RT_OK; }
  rtError m32(float& v) const { v = mMatrix.constData(9); return RT_OK; }
  rtError m33(float& v) const { v = mMatrix.constData(10); return RT_OK; }
  rtError m34(float& v) const { v = mMatrix.constData(11); return RT_OK; }
  rtError m41(float& v) const { v = mMatrix.constData(12); return RT_OK; }
  rtError m42(float& v) const { v = mMatrix.constData(13); return RT_OK; }
  rtError m43(float& v) const { v = mMatrix.constData(14); return RT_OK; }
  rtError m44(float& v) const { v = mMatrix.constData(15); return RT_OK; }

  rtError setM11(const float& v) { cancelAnimation("m11",true); mMatrix.data()[0] = v; return RT_OK; }
  rtError setM12(const float& v) { cancelAnimation("m12",true); mMatrix.data()[1] = v; return RT_OK; }
  rtError setM13(const float& v) { cancelAnimation("m13",true); mMatrix.data()[2] = v; return RT_OK; }
  rtError setM14(const float& v) { cancelAnimation("m14",true); mMatrix.data()[3] = v; return RT_OK; }
  rtError setM21(const float& v) { cancelAnimation("m21",true); mMatrix.data()[4] = v; return RT_OK; }
  rtError setM22(const float& v) { cancelAnimation("m22",true); mMatrix.data()[5] = v; return RT_OK; }
  rtError setM23(const float& v) { cancelAnimation("m23",true); mMatrix.data()[6] = v; return RT_OK; }
  rtError setM24(const float& v) { cancelAnimation("m24",true); mMatrix.data()[7] = v; return RT_OK; }
  rtError setM31(const float& v) { cancelAnimation("m31",true); mMatrix.data()[8] = v; return RT_OK; }
  rtError setM32(const float& v) { cancelAnimation("m32",true); mMatrix.data()[9] = v; return RT_OK; }
  rtError setM33(const float& v) { cancelAnimation("m33",true); mMatrix.data()[10] = v; return RT_OK; }
  rtError setM34(const float& v) { cancelAnimation("m34",true); mMatrix.data()[11] = v; return RT_OK; }
  rtError setM41(const float& v) { cancelAnimation("m41",true); mMatrix.data()[12] = v; return RT_OK; }
  rtError setM42(const float& v) { cancelAnimation("m42",true); mMatrix.data()[13] = v; return RT_OK; }
  rtError setM43(const float& v) { cancelAnimation("m43",true); mMatrix.data()[14] = v; return RT_OK; }
  rtError setM44(const float& v) { cancelAnimation("m44",true); mMatrix.data()[15] = v; return RT_OK; }

  rtError useMatrix(bool& v) const { v = mUseMatrix; return RT_OK; }
  rtError setUseMatrix(const bool& v) { mUseMatrix = v; return RT_OK; }

  void repaint() { mRepaint = true; mRepaintCount = 0; }

public:
  rtEmitRef mEmit;

protected:
  // TODO getting freaking huge... 
  rtRefT<pxObject> mParent;
  vector<rtRefT<pxObject> > mChildren;
//  vector<animation> mAnimations;
  float mcx, mcy, mx, my, ma, mr;
#ifdef ANIMATION_ROTATE_XYZ
  float mrx, mry, mrz;
#endif // ANIMATION_ROTATE_XYZ
  float msx, msy, mw, mh;
  bool mInteractive;
  pxContextFramebufferRef mSnapshotRef;
  bool mPainting;
  bool mClip;
  bool mMask;
  bool mDraw;
  bool mHitTest;
  rtObjectRef mReady;
  bool mFocus;
  pxContextFramebufferRef mClipSnapshotRef;
  bool mCancelInSet;
  rtString mId;
  pxMatrix4f mMatrix;
  bool mUseMatrix;
  bool mRepaint;
  int mRepaintCount;
  #ifdef PX_DIRTY_RECTANGLES
  bool mIsDirty;
  pxMatrix4f mLastRenderMatrix;
  pxRect mScreenCoordinates;
  #endif //PX_DIRTY_RECTANGLES

  pxContextFramebufferRef createSnapshot(pxContextFramebufferRef fbo);
  void createSnapshotOfChildren(pxContextFramebufferRef drawableFbo, pxContextFramebufferRef maskFbo);
  void deleteSnapshot(pxContextFramebufferRef fbo);
  #ifdef PX_DIRTY_RECTANGLES
  pxRect getBoundingRectInScreenCoordinates();
  #endif //PX_DIRTY_RECTANGLES

  pxScene2d* mScene;

 private:
  rtError _pxObject(voidPtr& v) const {
    v = (void*)this;
    return RT_OK;
  }
};

class pxViewContainer: public pxObject, public pxIViewContainer
{
public:
  rtDeclareObject(pxViewContainer, pxObject);
//  rtProperty(uri, uri, setURI, rtString);
  rtProperty(w, w, setW, float);
  rtProperty(h, h, setH, float);
  rtMethod1ArgAndNoReturn("onMouseDown", onMouseDown, rtObjectRef);
  rtMethod1ArgAndNoReturn("onMouseUp", onMouseUp, rtObjectRef);
  rtMethod1ArgAndNoReturn("onMouseMove", onMouseMove, rtObjectRef);
  rtMethod1ArgAndNoReturn("onMouseEnter", onMouseEnter, rtObjectRef);
  rtMethod1ArgAndNoReturn("onMouseLeave", onMouseLeave, rtObjectRef);
  rtMethod1ArgAndNoReturn("onFocus", onFocus, rtObjectRef);
  rtMethod1ArgAndNoReturn("onBlur", onBlur, rtObjectRef);
  rtMethod1ArgAndNoReturn("onKeyDown", onKeyDown, rtObjectRef);
  rtMethod1ArgAndNoReturn("onKeyUp", onKeyUp, rtObjectRef);
  rtMethod1ArgAndNoReturn("onChar", onChar, rtObjectRef);

  pxViewContainer(pxScene2d* scene):pxObject(scene)
  {
    addListener("onMouseDown", get<rtFunctionRef>("onMouseDown"));
    addListener("onMouseUp", get<rtFunctionRef>("onMouseUp"));
    addListener("onMouseMove", get<rtFunctionRef>("onMouseMove"));
    addListener("onMouseEnter", get<rtFunctionRef>("onMouseEnter"));
    addListener("onMouseLeave", get<rtFunctionRef>("onMouseLeave"));
    addListener("onFocus", get<rtFunctionRef>("onFocus"));
    addListener("onBlur", get<rtFunctionRef>("onBlur"));
    addListener("onKeyDown", get<rtFunctionRef>("onKeyDown"));
    addListener("onKeyUp", get<rtFunctionRef>("onKeyUp"));
    addListener("onChar", get<rtFunctionRef>("onChar"));
  }

  virtual ~pxViewContainer() {}

  rtError setView(pxIView* v)
  {
    mView = v;
    if (mView)
    {
      mView->onSize(mw,mh);
      mView->setViewContainer(this);
    }
    return RT_OK;
  }

  void invalidateRect(pxRect* r);

#if 0
  rtError url(rtString& v) const { v = mUri; return RT_OK; }
  rtError setUrl(rtString v) { mUri = v; return RT_OK; }
#endif

  rtError w(float& v) const { v = mw; return RT_OK; }
  rtError setW(float v) 
  { 
    mw = v; 
    if (mView)
      mView->onSize(mw,mh); 
    return RT_OK; 
  }
  
  rtError h(float& v) const { v = mh; return RT_OK; }
  rtError setH(float v) 
  { 
    mh = v; 
    if (mView)
      mView->onSize(mw,mh); 
    return RT_OK; 
  }

  rtError onMouseDown(rtObjectRef o)
  {
    rtLogDebug("pxViewContainer::onMouseDown");
    if (mView)
    {
      float x = o.get<float>("x");
      float y = o.get<float>("y");
      uint32_t flags = o.get<uint32_t>("flags");
      mView->onMouseDown(x,y,flags);
    }
    return RT_OK;
  }

  rtError onMouseUp(rtObjectRef o)
  {
    rtLogDebug("pxViewContainer::onMouseUp");
    if (mView)
    {
      float x = o.get<float>("x");
      float y = o.get<float>("y");
      uint32_t flags = o.get<uint32_t>("flags");
      mView->onMouseUp(x,y,flags);
    }
    return RT_OK;
  }

  rtError onMouseMove(rtObjectRef o)
  {
    rtLogDebug("pxViewContainer::onMouseMove");
    if (mView)
    {
      float x = o.get<float>("x");
      float y = o.get<float>("y");
      mView->onMouseMove(x,y);
    }
    return RT_OK;
  }

  rtError onMouseEnter(rtObjectRef /*o*/)
  {
    if (mView)
      mView->onMouseEnter();
    return RT_OK;
  }

  rtError onMouseLeave(rtObjectRef /*o*/)
  {
    if (mView)
      mView->onMouseLeave();
    return RT_OK;
  }

  rtError onFocus(rtObjectRef /*o*/)
  {
    if (mView) {
      mView->onFocus();
    }
    return RT_OK;
  }
  rtError onBlur(rtObjectRef /*o*/)
  {
    if (mView) {
      mView->onBlur();
    }
    return RT_OK;
  }
  rtError onKeyDown(rtObjectRef o)
  {
    if (mView)
    {
      uint32_t keyCode = o.get<uint32_t>("keyCode");
      uint32_t flags = o.get<uint32_t>("flags");
      mView->onKeyDown(keyCode, flags);
    }
    return RT_OK;
  }

  rtError onKeyUp(rtObjectRef o)
  {
    if (mView)
    {
      uint32_t keyCode = o.get<uint32_t>("keyCode");
      uint32_t flags = o.get<uint32_t>("flags");
      mView->onKeyUp(keyCode, flags);
    }
    return RT_OK;
  }

  rtError onChar(rtObjectRef o)
  {
    if (mView)
    {
      uint32_t codePoint = o.get<uint32_t>("charCode");
      mView->onChar(codePoint);
    }
    return RT_OK;
  }

  virtual void update(double t)
  {
    if (mView)
      mView->onUpdate(t);
    pxObject::update(t);
  }

  virtual void draw() 
  {
    if (mView)
      mView->onDraw();
  }

  

protected:
  pxViewRef mView;
  rtString mUrl;
};


class pxSceneContainer: public pxViewContainer
{
public:
  rtDeclareObject(pxSceneContainer, pxViewContainer);
  rtProperty(url, url, setUrl, rtString);
  rtReadOnlyProperty(api, api, rtValue);
  rtMethod1ArgAndNoReturn("makeReady", makeReady, bool);
  
pxSceneContainer(pxScene2d* scene):pxViewContainer(scene){}

  rtError url(rtString& v) const { v = mUrl; return RT_OK; }
  rtError setUrl(rtString v);

  rtError api(rtValue& v) const;

  rtError makeReady(bool ready);

  // in the case of pxSceneContainer, the makeReady should be the  
  // catalyst for ready to fire, so override sendPromise and 
  // createNewPromise to prevent firing from update() 
  virtual void sendPromise() { rtLogDebug("pxSceneContainer ignoring sendPromise\n"); }
  virtual void createNewPromise(){ rtLogDebug("pxSceneContainer ignoring createNewPromise\n"); }
  
private:
  rtRefT<pxScene2d> mScene;
  rtString mUrl;
};


typedef rtRefT<pxObject> pxObjectRef;

class pxScene2d: public rtObject, public pxIView {
public:
  rtDeclareObject(pxScene2d, rtObject);
  rtProperty(onScene, onScene, setOnScene, rtFunctionRef);
  rtReadOnlyProperty(root, root, rtObjectRef);
  rtReadOnlyProperty(w, w, int32_t);
  rtReadOnlyProperty(h, h, int32_t);
  rtProperty(showOutlines, showOutlines, setShowOutlines, bool);
  rtProperty(showDirtyRect, showDirtyRect, setShowDirtyRect, bool);
  rtMethod1ArgAndReturn("loadArchive",loadArchive,rtString,rtObjectRef); 
  rtMethod1ArgAndReturn("create", create, rtObjectRef, rtObjectRef);
  rtMethodNoArgAndReturn("clock", clock, uint64_t);
/*
  rtMethod1ArgAndReturn("createExternal", createExternal, rtObjectRef,
                        rtObjectRef);
  rtMethod1ArgAndReturn("createWayland", createWayland, rtObjectRef,
                        rtObjectRef);
*/
  rtMethod2ArgAndNoReturn("on", addListener, rtString, rtFunctionRef);
  rtMethod2ArgAndNoReturn("delListener", delListener, rtString, rtFunctionRef);

  // TODO make this a property
  // focus is now a bool property on pxObject
  //rtMethod1ArgAndNoReturn("setFocus", setFocus, rtObjectRef);
  rtMethodNoArgAndReturn("getFocus", getFocus, rtObjectRef);
  
  
  rtMethodNoArgAndNoReturn("stopPropagation",stopPropagation);
  
  rtMethod1ArgAndReturn("screenshot", screenshot, rtString, rtString);

  rtProperty(ctx, ctx, setCtx, rtValue);
  rtProperty(api, api, setAPI, rtValue);
//  rtReadOnlyProperty(emit, emit, rtFunctionRef);
  // Properties for returning various CONSTANTS
  rtReadOnlyProperty(animation,animation,rtObjectRef);
  rtReadOnlyProperty(stretch,stretch,rtObjectRef);
  rtReadOnlyProperty(alignVertical,alignVertical,rtObjectRef);
  rtReadOnlyProperty(alignHorizontal,alignHorizontal,rtObjectRef);
  rtReadOnlyProperty(truncation,truncation,rtObjectRef);

  pxScene2d(bool top = true);
  
  virtual unsigned long AddRef() {
    return rtAtomicInc(&mRefCount);
  }
  
  virtual unsigned long Release() {
    long l = rtAtomicDec(&mRefCount);
    if (l == 0) delete this;
    return l;
  }

  void init();

  rtError onScene(rtFunctionRef& v) const;
  rtError setOnScene(rtFunctionRef v);

  int32_t w() const { return mWidth;  }
  rtError w(int32_t& v) const { v = mWidth;  return RT_OK; }
  int32_t h() const { return mHeight; }
  rtError h(int32_t& v) const { v = mHeight; return RT_OK; }

  rtError showOutlines(bool& v) const;
  rtError setShowOutlines(bool v);

  rtError showDirtyRect(bool& v) const;
  rtError setShowDirtyRect(bool v);

  rtError create(rtObjectRef p, rtObjectRef& o);

  rtError createObject(rtObjectRef p, rtObjectRef& o);
  rtError createRectangle(rtObjectRef p, rtObjectRef& o);
  rtError createText(rtObjectRef p, rtObjectRef& o);
  rtError createTextBox(rtObjectRef p, rtObjectRef& o);
  rtError createImage(rtObjectRef p, rtObjectRef& o);
  rtError createImage9(rtObjectRef p, rtObjectRef& o);
  rtError createImageResource(rtObjectRef p, rtObjectRef& o); 
  rtError createFontResource(rtObjectRef p, rtObjectRef& o);  
  rtError createScene(rtObjectRef p,rtObjectRef& o);
  rtError createExternal(rtObjectRef p, rtObjectRef& o);

  rtError clock(uint64_t & time);
  rtError createWayland(rtObjectRef p, rtObjectRef& o);

  rtError addListener(rtString eventName, const rtFunctionRef& f)
  {
    return mEmit->addListener(eventName, f);
  }

  rtError delListener(rtString  eventName, const rtFunctionRef& f)
  {
    return mEmit->delListener(eventName, f);
  }

  rtError getFocus(rtObjectRef& o)
  {
    o = mFocusObj;
    return RT_OK;
  }

  rtError setFocus(rtObjectRef o);
 
  rtError stopPropagation()
  {
    printf("stopPropagation()\n");
    mStopPropagation = true;
    return RT_OK;
  }

  rtError ctx(rtValue& v) const { v = mContext; return RT_OK; }
  rtError setCtx(const rtValue& v) { mContext = v; return RT_OK; }

  rtError api(rtValue& v) const { v = mAPI; return RT_OK; }
  rtError setAPI(const rtValue& v) { mAPI = v; return RT_OK; }

  rtError emit(rtFunctionRef& v) const { v = mEmit; return RT_OK; }
  
  rtError animation(rtObjectRef& v) const {v = CONSTANTS.animationConstants; return RT_OK;}
  rtError stretch(rtObjectRef& v) const {v = CONSTANTS.stretchConstants; return RT_OK;}
  rtError alignVertical(rtObjectRef& v) const {v = CONSTANTS.alignVerticalConstants; return RT_OK;}
  rtError alignHorizontal(rtObjectRef& v) const {v = CONSTANTS.alignHorizontalConstants; return RT_OK;}
  rtError truncation(rtObjectRef& v) const {v = CONSTANTS.truncationConstants; return RT_OK;}

  void setMouseEntered(pxObject* o);

  // The following methods are delegated to the view
  virtual void onSize(int32_t w, int32_t h);

  virtual void onMouseDown(int32_t x, int32_t y, uint32_t flags);
  virtual void onMouseUp(int32_t x, int32_t y, uint32_t flags);
  virtual void onMouseEnter();
  virtual void onMouseLeave();
  virtual void onMouseMove(int32_t x, int32_t y);

  virtual void onFocus();
  virtual void onBlur();

  virtual void onKeyDown(uint32_t keycode, uint32_t flags);
  virtual void onKeyUp(uint32_t keycode, uint32_t flags);
  virtual void onChar(uint32_t codepoint);
  
  virtual void onUpdate(double t);
  virtual void onDraw();

  virtual void setViewContainer(pxIViewContainer* l) 
  {
    mContainer = l;
  }

  void invalidateRect(pxRect* r);
  
  void getMatrixFromObjectToScene(pxObject* o, pxMatrix4f& m);
  void getMatrixFromSceneToObject(pxObject* o, pxMatrix4f& m);
  void getMatrixFromObjectToObject(pxObject* from, pxObject* to, pxMatrix4f& m);
  void transformPointFromObjectToScene(pxObject* o, const pxPoint2f& from, 
				       pxPoint2f& to);
  void transformPointFromSceneToObject(pxObject* o, const pxPoint2f& from, pxPoint2f& to);
  void transformPointFromObjectToObject(pxObject* fromObject, pxObject* toObject,
					pxPoint2f& from, pxPoint2f& to);
  
  void hitTest(pxPoint2f p, vector<rtRefT<pxObject> > hitList);
  
  pxObject* getRoot() const;
  rtError root(rtObjectRef& v) const 
  {
    v = getRoot();
    return RT_OK;
  }

  rtError loadArchive(const rtString& url, rtObjectRef& archive)
  {
    rtError e = RT_FAIL;
    printf("1\n");
    rtRefT<pxArchive> a = new pxArchive;
    printf("2\n");
    if (a->initFromUrl(url) == RT_OK)
    {
    printf("3\n");
      archive = a;
      e = RT_OK;
    }
    return e;
  }
  
private:
  void bubbleEvent(rtObjectRef e, rtRefT<pxObject> t, 
                   const char* preEvent, const char* event) ;

  void draw();
  // Does not draw updates scene to time t
  // t is assumed to be monotonically increasing
  void update(double t);

  // Note: Only type currently supported is "image/png;base64"
  rtError screenshot(rtString type, rtString& pngData);
  
  rtRefT<pxObject> mRoot;
  rtObjectRef mFocusObj;
  double start, end2;
  int frameCount;
  int mWidth;
  int mHeight;
  rtEmitRef mEmit;

// TODO Top level scene only
  rtRefT<pxObject> mMouseEntered;
  rtRefT<pxObject> mMouseDown;
  pxPoint2f mMouseDownPt;
  rtValue mContext;
  rtValue mAPI;
  bool mTop;
  bool mStopPropagation;
  int mTag;
  pxIViewContainer *mContainer;
  bool mShowDirtyRect;
  #ifdef USE_SCENE_POINTER
  pxTextureRef mNullTexture;
  pxTextureCacheObject mPointerTextureCacheObj;
  pxTextureRef mPointerTexture;
  int32_t mPointerX;
  int32_t mPointerY;
  int32_t mPointerW;
  int32_t mPointerH;
  int32_t mPointerHotSpotX;
  int32_t mPointerHotSpotY;
  #endif
  bool mPointerHidden;
public:
  void hidePointer( bool hide )
  {
     mPointerHidden= hide;
  }
  bool mDirty;
  #ifdef PX_DIRTY_RECTANGLES
  static pxRect mDirtyRect;
  #endif //PX_DIRTY_RECTANGLES
};

// TODO do we need this anymore?
class pxScene2dRef: public rtRefT<pxScene2d>, public rtObjectBase
{
 public:
  pxScene2dRef() {}
  pxScene2dRef(pxScene2d* s) { asn(s); }

  // operator= is not inherited
  pxScene2dRef& operator=(pxScene2d* s) { asn(s); return *this; }
  
 private:
  virtual rtError Get(const char* name, rtValue* value) const;
  virtual rtError Get(uint32_t i, rtValue* value) const;
  virtual rtError Set(const char* name, const rtValue* value);
  virtual rtError Set(uint32_t i, const rtValue* value);
};


#endif
