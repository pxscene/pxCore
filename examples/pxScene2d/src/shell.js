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

var isDuk=(typeof Duktape != "undefined")?true:false;
var isV8 = (typeof _isV8 != "undefined")?true:false;

var defaultAppUrl = 'browser.js'

px.import({ scene: 'px:scene.1.js',
  log: "px:tools.../Logger.js",
  keys: 'px:tools.keys.js',
  url: 'url',
  querystring: 'querystring'
}).then( function ready(imports)
{
  var scene = imports.scene;
  var keys  = imports.keys;
  var urlModule = imports.url;
  var queryStringModule = imports.querystring;
  var Logger = imports.log.Logger;
  var setLoggingLevel = imports.log.setLoggingLevel;

  var logger = new Logger('shell.js');
  var rtLogLevel = process.env.RT_LOG_LEVEL ?  process.env.RT_LOG_LEVEL : 'warn';
  setLoggingLevel(rtLogLevel);

  var appUrl = scene.sparkSetting('defaultAppUrl')
  defaultAppUrl = appUrl?appUrl:defaultAppUrl

  /**
   * This is helper method which resolves resource URL for scene
   * - it resolves various shortcuts using prepareUrl() method
   * also
   * - for .js files it returns URL as it is
   * - for other files (MIME files) it returns the URL of wrapper scene which
   *   will draw provided URL with the mimeRenderer
   *
   * @param {String} url url
   *
   * @returns {String} URL for a scene
   */
  /*
  function resolveSceneUrl(url) {
    if (url) {
      let pos = url.indexOf('#');
      let noQueryPart;
      if (pos !== -1) {
        noQueryPart = url.substring(0, pos);
      } else {
        noQueryPart = url;
      }
      pos = noQueryPart.indexOf('?');
      if (pos !== -1) {
        noQueryPart = noQueryPart.substring(0, pos);
      }
      if (!/.js$/.test(noQueryPart) && !/.mjs$/.test(noQueryPart) && !/.spark$/.test(noQueryPart)) {
        url = 'mimeScene.js?url=' + url;
      }
    }
    return url;
  }
  */

  // JRJR TODO had to add more modules
  var url = queryStringModule.parse(urlModule.parse(module.appSceneContext.packageUrl).query).url;
  //url = resolveSceneUrl(url)
  var originalURL = (!url || url==="") ? defaultAppUrl:url;
  logger.message('info', "url:" + originalURL);

  var    blackBg = scene.create({t:"rect", fillColor:0x000000ff,x:0,y:0,w:1280,h:720,a:0,parent:scene.root});
  var defaultToOpaqueBackground = true;
  if (originalURL.startsWith("gl:") || originalURL.includes(".spark"))
  {
    blackBg.a = 0;
    defaultToOpaqueBackground = false;
  }
  var childScene = scene.create({t:"scene", url: originalURL, parent:scene.root});
  childScene.focus = true;

  var showFPS = false;
  var fpsBg      = scene.create({t:"rect",     fillColor:0x00000080,lineColor:0xffff0080,lineWidth:3,x:10,y:10,a:showFPS?1:0,parent:scene.root});
  var fpsCounter = scene.create({t:"text", x:5,textColor:0xffffffff,pixelSize:24,text:"0fps",parent:fpsBg});
  fpsBg.w = fpsCounter.w+16;
  fpsBg.h = fpsCounter.h;

  // Prevent interaction with scenes...
  fpsBg.interactive      = false;
  fpsCounter.interactive = false;

  function updateSize(w, h)
  {
    childScene.w = w;
    childScene.h = h;
    blackBg.w    = w;
    blackBg.h    = h;
  }

  // TODO if I log out event object e... there is extra stuff??
  scene.on("onResize", function(e) { updateSize(e.w, e.h);});

  // updateSize(scene.w, scene.h);

  scene.on("onFPS", function(e) {
    if(showFPS) {
      fpsCounter.text = ""+Math.floor(e.fps)+"fps";
      fpsBg.w = fpsCounter.w+16;
      fpsBg.h = fpsCounter.h;
    }
  });

////
if( scene.capabilities != undefined && scene.capabilities.graphics != undefined && scene.capabilities.graphics.cursor == 1) {
  // TODO Cursor emulation mostly for egl targets right now.
    var fadeCursorTimer = undefined;

    var cursor = scene.create({t:"image", url:"cursor.png",parent:scene.root,
                               interactive:false, a:0});

    scene.on("onMouseMove", function(e) {
      if (fadeCursorTimer != undefined) {
        clearTimeout(fadeCursorTimer);
      }
      fadeCursorTimer = setTimeout( function()
      {
        cursor.animate({a:0}, 2, scene.animation.TWEEN_LINEAR, scene.animation.LOOP,1);
      }, 5000);
      cursor.a = 1.0;
      cursor.x = e.x-40;
      cursor.y = e.y-16;
    });
}
////

  scene.root.on("onPreKeyDown", function(e) {
    var code  = e.keyCode;
    var flags = e.flags;

    var loggingDisabled = process.env.PXSCENE_KEY_LOGGING_DISABLED;
    if (loggingDisabled && loggingDisabled === '1'){
      logger.message('warn', "onPreKeyDown value hidden");
    } else {
      logger.message('info', "SHELL: onPreKeyDown: " + code + " key: " + keys.name(code) + ", ", flags);
    }

    if( keys.is_CTRL_ALT( flags ) )
    {
      if(code == keys.Y)  // ctrl-alt-y
      {
//        logger.message('info', "SHELL: onPreKeyDown: FPS !!!  ############# ");

        showFPS = !showFPS;
        scene.reportFps = showFPS;
        fpsBg.a = (showFPS)?1.0:0;
        e.stopPropagation();
      }
      else
      if(code == keys.O)  // ctrl-alt-o
      {
//        logger.message('info', "SHELL: onPreKeyDown: showOutlines !!!  ############# ");

        scene.showOutlines = !scene.showOutlines;
        e.stopPropagation();
      }
      else
      if (code == keys.S)  // ctrl-alt-s
      {
        if (!isDuk && !isV8) {
        // This returns a data URI string with the image data
        var dataURI = scene.screenshot('image/png;base64');

        // convert the data URI by stripping off the scheme and type information
        // to a base64 encoded string with just the PNG image data
        var base64PNGData = dataURI.slice(dataURI.indexOf(',')+1);

        // decode the base64 data and write it to a file
        fs.writeFile("screenshot.png", new Buffer(base64PNGData, 'base64'), function(err)
        {
          if (err)
            console.log("Error creating screenshot.png");
          else
            console.log("Created screenshot.png");
        });
      }
        e.stopPropagation();
      }
      else
      if(code == keys.D)  // ctrl-alt-d
      {
        // logger.message('info', "SHELL: onPreKeyDown: show dirty rect !!!  ############# ");

        scene.showDirtyRect = !scene.showDirtyRect;
        e.stopPropagation();
      }
      else
      if(code == keys.T)  // ctrl-alt-t
      {
        scene.enableDirtyRect = !scene.enableDirtyRect;
        e.stopPropagation();
      }
    }// ctrl-alt
    else
    if( keys.is_CTRL_ALT_SHIFT( flags ) )
    {
      if(code == keys.R)  // ctrl-alt-shft-r
      {
        // logger.message('info', "SHELL: onPreKeyDown: Reloading url [ "+originalURL+" ] !!!  ############# ");

        logger.message('warn',"Reloading url: " + originalURL);
        childScene.url = originalURL;
        e.stopPropagation();
      }
      else
      if(code == keys.H)  // ctrl-alt-shft-h
      {
        // console.log("SHELL: onPreKeyDown: Loading HOME url [ "+defaultAppUrl+" ] !!!  ############# ");

        var homeURL = defaultAppUrl;
        logger.message('warn',"Loading home url: " + homeURL);
        childScene.url = homeURL;
        e.stopPropagation();
      }
      else
      if(code == keys.D)  // ctrl-alt-shft-d
      {
        scene.logDebugMetrics();
      }
    }// ctrl-alt-shift
  });

  scene.root.on("onPreKeyUp", function(e)
  {
    var loggingDisabled = process.env.PXSCENE_KEY_LOGGING_DISABLED;
    if (loggingDisabled && loggingDisabled === '1'){
      logger.message('warn', "onPreKeyUp value hidden");
    } else {
      logger.message('info', "in onPreKeyUp " +  e.keyCode + ", " + e.flags);
    }
    var code  = e.keyCode;
    var flags = e.flags;

    if (loggingDisabled && loggingDisabled === '1'){
      logger.message('warn', "onKeyUp value hidden");
    } else {
      logger.message('info', "onKeyUp:" + code, ", " + flags);
    }

    // eat the ones we handle here
         if (code == keys.Y && keys.is_CTRL_ALT( flags ) )       e.stopPropagation(); // ctrl-alt-y
    else if (code == keys.O && keys.is_CTRL_ALT( flags ) )       e.stopPropagation(); // ctrl-alt-o
    else if (code == keys.S && keys.is_CTRL_ALT( flags ) )       e.stopPropagation(); // ctrl-alt-s
    else if (code == keys.D && keys.is_CTRL_ALT( flags ) )       e.stopPropagation(); // ctrl-alt-d
    else if (code == keys.T && keys.is_CTRL_ALT( flags ) )       e.stopPropagation(); // ctrl-alt-t
    else if (code == keys.R && keys.is_CTRL_ALT_SHIFT( flags ) ) e.stopPropagation(); // ctrl-alt-shift-r
    else if (code == keys.H && keys.is_CTRL_ALT_SHIFT( flags ) ) e.stopPropagation(); // ctrl-alt-shift-h
  });

  if (true)
  {
    scene.root.on("onKeyDown", function(e)
    {
      var code = e.keyCode; var flags = e.flags;
      var loggingDisabled = process.env.PXSCENE_KEY_LOGGING_DISABLED;
      if (loggingDisabled && loggingDisabled === '1'){
        logger.message('warn', "onKeyDown value hidden");
      } else {
        logger.message('info', "onKeyDown shell: " + code, ", " + flags);
      }

      if( keys.is_CTRL_ALT( flags ) )
      {
        if(code == keys.R)   // ctrl-alt-r
        {
          logger.message('warn', "(shell.js) Reloading url: " + originalURL);
          childScene.url = originalURL;
          e.stopPropagation();
        }
        else
        if (code == keys.H)  // ctrl-alt-h
        {
          var homeURL = defaultAppUrl;
          logger.message('warn', "Loading home url: " + homeURL);
          childScene.url = homeURL;
          e.stopPropagation();
        }
      }// ctrl-alt
    });
  }

  scene.root.on("onPreChar", function(e)
  {
    logger.message('debug', "in onchar");
    var c = e.charCode;
    var loggingDisabled = process.env.PXSCENE_KEY_LOGGING_DISABLED;
    if (loggingDisabled && loggingDisabled === '1'){
      logger.message('warn', "onChar value hidden");
    } else {
      logger.message('info', "onChar: " + c);
    }
    // TODO eating some "undesired" chars for now... need to redo this
    if (c<32) {
      logger.message('debug', "stop onChar");
      e.stopPropagation();
    }
  });

  // TODO if I log out event object ... there is extra stuff??
  scene.on("onResize", function(e) { updateSize(e.w, e.h);});
  updateSize(scene.w, scene.h);

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Test for Clearscreen delegate...
  childScene.ready.then( function()
  {
      // Use delegate if present...
      //
      if (typeof childScene.api                  !== undefined &&
          typeof childScene.api.wantsClearscreen === 'function')
      {
        if(childScene.api.wantsClearscreen()) // use delegate preference - returns bool
          blackBg.a = 1;

      }
      else if (defaultToOpaqueBackground) {
          blackBg.a = 1;
      }
  });
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Example of 'delegate' for childScene.
  //
  // Returns:   BOOL preference of 'shell' performing Clearscreen per frame.
  /*
  module.exports.wantsClearscreen = function()  // delegate
  {
    return false;
  };
  */
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  function releaseResources() {

  }

  scene.on("onClose",releaseResources);
});
