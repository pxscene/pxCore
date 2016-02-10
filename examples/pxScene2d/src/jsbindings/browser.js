px.import("px:scene.1.js").then( function ready(scene) {
var root = scene.root;
var bg = scene.create({t:"image",url:"../../images/status_bg.png",parent:root,xStretch:1,yStretch:1});
var inputbg = scene.create({t:"image9",a:0.9,url:"../../images/input2.png",x:10,y:10,w:400,lInset:10,rInset:10,tInset:10,bInset:10,parent:bg});
var prompt = scene.create({t:"text",text:"Enter URL to JS File or Package",parent:inputbg,pixelSize:24,textColor:0x869CB2ff,x:10,y:2});
var url = scene.create({t:"text",text:"",parent:inputbg,pixelSize:24,textColor:0x303030ff,x:10,y:2});
var cursor = scene.create({t:"rect", w:2, h:inputbg.h-10, parent:inputbg,x:10,y:5});

//var wayland = scene.create({t:"wayland", displayName:"nested0", x:200, y:200, w:1280, h:720, parent:root, cmd:"/home/jwanna001/projects/wayland/weston/weston-terminal" });

cursor.animateTo({a:0},0.5,scene.PX_LINEAR,scene.PX_SEESAW);


var contentBG = scene.create({t:"rect",x:10,y:60,parent:bg,fillColor:0xffffffff,a:0.05,draw:false});
var content = scene.createScene({x:10,y:60,parent:bg,clip:true});

//var wayland = scene.createWayland( {displayName:"nested0", width:640, height:360, parent:root} );

var txt1= scene.createText( {x:60, y:50, pixelSize: 30, h:40, text:"", parent:root} );
var txt2= scene.createText( {x:60, y:90, pixelSize: 30, h:35, text:"", parent:root} );
var txt3= scene.createText( {x:640, y:50, pixelSize: 30, h:40, text:"", parent:root} );
var txt4= scene.createText( {x:640, y:90, pixelSize: 30, h:35, text:"", parent:root} );

var wayland1, wayland2;

setTimeout(function(){

  wayland1 = scene.create( {t:"wayland", displayName:"nested2", x:200, y:200, w:1280, h:720, parent:root, cmd:"/home/jwanna001/projects/wayland/weston/weston-terminal"} );

  wayland1.on("onClientConnected", function(e) { txt1.text="Client connected: pid: "+e.pid; txt2.text="" });
  wayland1.on("onClientDisconnected", function(e) { txt1.text="Client disconnected: pid: "+e.pid; });
  wayland1.on("onClientStopped", function(e) { 
    if ( e.crashed == true ) {
       txt2.text="Client crashed: pid: "+e.pid+" signo:"+e.signo; 
       txt2.textColor=0xFF8080FF;
    } else {
       txt2.text="Client ended normally: pid: "+e.pid+" code:"+e.exitCode; 
       txt2.textColor=0x20FF20FF; 
    }
  });
  
  scene.setFocus(wayland1);
 
}, 4000 );

setTimeout(function(){
   wayland2 = scene.create( {t:"wayland", x:640, y:360, w:640, h:360, r:-20, parent:root, cmd:"/home/jwanna001/projects/wayland/weston/weston-clickdot"} );

   wayland2.on("onClientConnected", function(e) { txt3.text="Client connected: pid: "+e.pid; txt2.text="" });
   wayland2.on("onClientDisconnected", function(e) { txt3.text="Client disconnected: pid: "+e.pid; });
   wayland2.on("onClientStopped", function(e) { 
    if ( e.crashed == true ) {
       txt4.text="Client crashed: pid: "+e.pid+" signo:"+e.signo; 
       txt4.textColor=0xFF8080FF;
    } else {
       txt4.text="Client ended normally: pid: "+e.pid+" code:"+e.exitCode; 
       txt4.textColor=0x20FF20FF; 
    }
  });
}, 10000 );


inputbg.on("onChar",function(e) {
  if (e.charCode == 13) 
    return;
  // TODO should we be getting an onChar event for backspace
  if (e.charCode != 8) {
    url.text += String.fromCharCode(e.charCode);
    prompt.a = (url.text)?0:1;
    cursor.x = url.x+url.w;
  }
});

inputbg.on("onKeyDown", function(e) {
  if (e.keyCode == 13) {

    var u = url.text;
    // TODO Temporary hack
    if (u.indexOf(':') == -1)
      u = 'http://www.pxscene.org/examples/px-reference/gallery/' + u;

    content.url = u;
    scene.setFocus(content);
    content.ready.then(function() {
      contentBG.draw = true;
/*
      console.log("api after promise:"+content.api);
      if (content.api) {
        content.api.test("john");
        content.api.middle.fillColor=0x000000ff;
      }
*/
    },
                       function() {
                         contentBG.draw = false;
                         console.log("scene load failed");
                       }
    );
  }
  else if (e.keyCode == 8) {
    var s = url.text.slice();
    console.log(url.text);
    console.log(s.slice(0,url.text.length-2));
//    url.text = s.slice(0,s.length-2);
    url.text = s.slice(0,-1);
  }
  prompt.a = (url.text)?0:1;
  cursor.x = url.x+url.w;
});

inputbg.on("onFocus", function(e) {
  cursor.draw = true;
});

inputbg.on("onBlur", function(e) {
  cursor.draw = false;
});

inputbg.on("onMouseUp", function(e) {
  scene.setFocus(inputbg);
});

content.on("onMouseUp", function(e) {
  scene.setFocus(content);
});

function updateSize(w,h) {
  bg.w = w;
  bg.h = h;
  inputbg.w = w-20;
  content.w = w-20;
  content.h = h-70;
  contentBG.w = w-20;
  contentBG.h = h-70;
}

scene.root.on("onPreKeyDown", function(e) {
  if (e.keyCode == 76 && e.flags == 16) { // ctrl-l
    //console.log("api:"+content.api);
//    if (content.api) content.api.test(32);
    scene.setFocus(inputbg);
    url.text = "";
    prompt.a = (url.text)?0:1;
    cursor.x = 10;
    e.stopPropagation();
  }
});

scene.on("onResize", function(e) { updateSize(e.w,e.h); });
updateSize(scene.w,scene.h);

//scene.setFocus(inputbg);
//scene.setFocus(wayland);

}).catch( function importFailed(err){
  console.error("Import failed for browser.js: " + err)
});

