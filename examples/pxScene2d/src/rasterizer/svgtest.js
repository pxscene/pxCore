

px.import({ scene:      'px:scene.1.js',
             keys:      'px:tools.keys.js',
}).then( function importsAreReady(imports)
{
  var scene = imports.scene;
  var keys  = imports.keys;
  var root  = imports.scene.root;
        
  var bg = scene.create({t:"rect",  parent: root, x:0, y:0, w: 100, h: 100, fillColor: 0x888888ff, a: 1.0 });

  bg.ready.then(
    function(o)
    {
      // Multi-color Cross ... (relative)
      if(false)
      {
        var sw = 6;
      
        scene.create( { t: "path", d:"M100 100 v-50", strokeColor: 0xFF00FFff, strokeWidth: sw, parent: bg} );
        scene.create( { t: "path", d:"M100 100 v50",  strokeColor: 0x00FF00ff, strokeWidth: sw, parent: bg} );
        scene.create( { t: "path", d:"M100 100 h50",  strokeColor: 0x0000FFff, strokeWidth: sw, parent: bg} );
        scene.create( { t: "path", d:"M100 100 h-50", strokeColor: 0x00FFFFff, strokeWidth: sw, parent: bg} );
      }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//     CIRCLE TESTING - see [test.svg]
//
//     <circle cx="50" cy="50" r="40" stroke="black" stroke-width="3" fill="red" />
//
//     <path d="M 191 140
//              C 191 163   173 181  150  181
//              C 127 181   109 163  109  140
//              C 109 117   127  99  150   99
//              C 173  99   191 117  191  140Z" stroke="black" stroke-width="3"  fill="cyan"/>
                
      if(true)
      {
        scene.create( { t: "path", d:"circle cx:150 cy:140 r:41", strokeColor: 0x000000ff, strokeWidth: 4, fillColor: 0x00FFFFff, parent: bg} );

        // Part of the converted arcs
//        scene.create( { t: "path", d:"M191 140 C 191 163 173 181  150  181", strokeColor: 0x000000ff, strokeWidth: 3, parent: bg} );
      }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//     ELLIPSE TESTING - see [test.svg]
//
//    <ellipse cx="60" cy="60" rx="50" ry="25" stroke="black" stroke-width="3" fill="red" />
//
//    <path d="M 200 340 C 200 354 178 365 150 365
//             C 122 365 100 354 100 340
//             C 100 326 122 315 150 315
//             C 178 315 200 326 200 340Z" stroke="black" stroke-width="3"  fill="cyan"/>
                
      if(true)
      {
        scene.create( { t: "path", d:"ellipse cx:150 cy:340 rx:50 ry: 25", strokeColor: 0x000000ff, strokeWidth: 3, fillColor: 0xFFFF00ff, parent: bg} );
      }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//    ARC: absolute
//
//    Example from: ARC section >>  https://developer.mozilla.org/en-US/docs/Web/SVG/Tutorial/Paths
//
      if(false)
      {
        scene.create( { t: "path", d:"M10 315 L 315 10", strokeColor: 0xFF0000ff, strokeWidth: 3,  parent: bg} );
        scene.create( { t: "path", d:"M10 315 "
                                    +"L 110 215 "
                                    +"A 30 50 0 0 1 162.55 162.45 "
                                    +"L 172.55 152.45 "
                                    +"A 30 50 -45 0 1 215.1 109.9 "
                                    +"L 315 10Z",strokeWidth: 2, strokeColor: 0x000000ff, fillColor: 0x00FF0080,  parent: bg} );
      }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//    ARC: relative
//
//    Example from: ARC section >>  https://www.w3.org/TR/SVG/paths.html#PathDataEllipticalArcCommands
//
      if(false)
      {
        scene.create( { t: "path", d:"M600 350 l 50 -25 "
                                    +"a25 25  -30 0 1 50 -25 l 50 -25 "
                                    +"a25 50  -30 0 1 50 -25 l 50 -25 "
                                    +"a25 75  -30 0 1 50 -25 l 50 -25 "
                                    +"a25 100 -30 0 1 50 -25 l 50 -25Z",
                     fillColor: 0x00000000, strokeColor: 0xFF0000ff, strokeWidth: "5",  parent: bg } );
      }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//    ARC SWEEP - 4 possible values
//
//    Example from: ARC section >>  https://developer.mozilla.org/en-US/docs/Web/SVG/Tutorial/Paths

      if(false)
      {
        scene.create( { t: "path", d: "M80 80 "
                                     +"A 45 45  0  0  0  125 125 "
                                     +"L 125 80 Z",
                     fillColor: 0x00FF00ff, strokeColor: 0x000000ff, strokeWidth: 1,  parent: bg } );

        scene.create( { t: "path", d: "M230 80 "
                                     +"A 45 45  0  1  0  275 125 "
                                     +"L 275 80 Z",
                     fillColor: 0xFF0000ff, strokeColor: 0x000000ff, strokeWidth: 1,  parent: bg } );

        scene.create( { t: "path", d: "M80 230 "
                                     +"A 45 45  0  0  1  125 275 "
                                     +"L 125 230 Z",
                     fillColor: 0xFF00FFff, strokeColor: 0x000000ff, strokeWidth: 1,  parent: bg } );

        scene.create( { t: "path", d: "M230 230 "
                                     +"A 45 45  0  1  1  275 275 "
                                     +"L 275 230 Z",
                     fillColor: 0x0000FFff, strokeColor: 0x000000ff, strokeWidth: 1,  parent: bg } );
      }
                
      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      //
      //    Curves
      //
      // These curves should overlap, Black Below, Red Above
      //
      if(false)
      {
        scene.create( { id: "CurveBG", t: "path", d:"M100 180 q50 10, 95 80",     strokeColor: 0x000000ff, strokeWidth: 13, parent: bg} );
        scene.create( { id: "CurveFG", t: "path", d:"M100 180 Q150 190, 195 260", strokeColor: 0xFF0000ff, strokeWidth: 3,  parent: bg} );
      }
    }
  );
//##################################################################################################################################
 
  bg.on("onMouseUp", function(e)
  {
    if(true)
    {
//##################################################################################################################################
//
//------ LINE/MOVE
//
        var ss = 2;
        
//      var shape1 = scene.create( { id: "Path (0,0)",  t: "path", d:"M0 0 l100 0 0 100 -100 0 0 -100", interactive: false,
//                                x: 0, y: 0, w: 100, h: 100, fillColor: 0x00FF0088, strokeColor: 0x000000ff, strokeWidth: ss, parent: bg, cx: 50, cy: 50} );
//      var shape2 = scene.create( { id: "Path (50,50)",  t: "path", d:"M0 0 l100 0 0 100 -100 0 0 -100", interactive: false,
//                                x: 50, y: 50, w: 100, h: 100, fillColor: 0x00FF0088, strokeColor: 0x000000ff, strokeWidth: ss, parent: bg, cx: 50, cy: 50} );
//      var shape3 = scene.create( { id: "Path (100,100)",  t: "path", d:"M0 0 l100 0 0 100 -100 0 0 -100", interactive: false,
//                                x: 100, y: 100, w: 100, h: 100, fillColor: 0x00FF0088, strokeColor: 0x000000ff, strokeWidth: ss, parent: bg, cx: 50, cy: 50} );
      
//       scene.create( { t: "path", d:"M100 100 L200 200",                         fillColor: 0x00FF88ff, parent: bg} );
//    var shape = scene.create( { t: "path", d:"M100 100 L200 100 200 200 100 200 100 100", strokeColor: 0xFF000Fff, strokeWidth: 6, parent: bg} );

//        var shape = scene.create( { id: "Path ("+e.x+","+e.y+")",  t: "path", d:"M0 0 l100 0 0 100 -100 0 0 -100", interactive: false,
//                                 x: e.x-50, y: e.y-50, w: 100, h: 100, fillColor: 0x00FF0088, strokeColor: 0x000000ff, strokeWidth: ss, parent: bg, cx: 50, cy: 50} );
        
//        var shape = scene.create( { id: "Path ("+e.x+","+e.y+")",  t: "path", d:"M" +(e.x-50)+ " " +(e.y-50)+ " l100 0 0 100 -100 0 0 -100", interactive: false,
//                                  w: 100, h: 100, fillColor: 0x00FF0088, strokeColor: 0x000000ff, strokeWidth: ss, parent: bg, cx: 50, cy: 50} );
        
      var shape = scene.create( { id: "Path - triangle ("+e.x+","+e.y+")",  t: "path", d:"M50 0 L100 100  0 100  50 0", interactive: false,
                             x: e.x-50, y: e.y-50, fillColor: 0xFF000088, strokeColor: 0x000000ff, strokeWidth: 6, parent: bg, cx: 50, cy: 50} );

//        var shape = scene.create( { t: "path", d:"circle cx:150 cy:140 r:50", x: e.x-50, y: e.y-50,cx: 50, cy: 50, strokeColor: 0x000000ff, strokeWidth: 4,  fillColor: 0x00FF00ff,  parent: bg} );
        
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //
        //  Animation
        //
        if(true)
        {
          shape.ready.then
          (
              function(o)
              {
                o.animateTo({ r: 360 }, 1.75, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_FASTFORWARD, 1);
               // var px = o.x + 100;
               // o.animateTo({ x: px }, 10.75, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_FASTFORWARD, 1);

               // var ww = o.w + 100;
               // o.animateTo({ w: ww }, 10.75, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_FASTFORWARD, 1);
              } );
        }
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
//        scene.create( { t: "path", d:"M100 100 v-50", x: 100, y:  50, w: 50, h: 50, strokeColor: 0xFF00FFff, strokeWidth: 4,  parent: bg} );
//        scene.create( { t: "path", d:"M100 100 v50",  x: 100, y: 150, w: 50, h: 50, strokeColor: 0x00FF00ff, strokeWidth: 4,  parent: bg} );
//        scene.create( { t: "path", d:"M100 100 h50",  x: 150, y: 100, w: 50, h: 50, strokeColor: 0x0000FFff, strokeWidth: 4,  parent: bg} );
//        scene.create( { t: "path", d:"M100 100 h-50", x:  50, y: 100, w: 50, h: 50, strokeColor: 0x00FFFFff, strokeWidth: 4,  parent: bg} );

//        scene.create( { t: "path", d:"M100 100 h-50 M100 100 v50 M100 100 h50 M100 100 v-50",  strokeColor: 0x00FF88ff, strokeWidth: 6, parent: bg} );  // relative

//##################################################################################################################################
//
//------ VERTICAL
//
//       scene.create( { t: "path", d:"M100 100 v-50",  strokeColor: 0x00FF88ff, strokeWidth: 6, parent: bg} );  // relative
//       scene.create( { t: "path", d:"V200", strokeColor: 0x00FF88ff, strokeWidth: 6, parent: bg} );  // absolute
             
//##################################################################################################################################
//
//------ HORIZONTAL
//
//       scene.create( { t: "path", d:"M100 100 h-50",  strokeColor: 0x00FF88ff, strokeWidth: 6, parent: bg} );  // relative
//       scene.create( { t: "path", d:"H200", strokeColor: 0x00FF88ff, strokeWidth: 6, parent: bg} );  // absolute
             
//##################################################################################################################################
//
//------ QUADRATIC CURVE
        
//        scene.create( { t: "path", d:"M10 80 Q 50 10, 95 80 T 180 80 270 110", strokeColor: 0x000000ff, strokeWidth: 3, parent: bg} );  // relative
//        scene.create( { t: "path", d:"M10 80 Q 50 10, 95 80 T 180 80", strokeColor: 0x000000ff, strokeWidth: 3, parent: bg} );  // relative

//         These curves should overlap, Black Below, Red Above
//        scene.create( { id: "CurveBG", t: "path", d:"M100 180 q50 10, 95 80",     strokeColor: 0x0000001f, strokeWidth: 9, parent: bg} );  // relative
//        scene.create( { id: "CurveFG", t: "path", d:"M100 180 Q150 190, 195 260", strokeColor: 0xFF0000ff, strokeWidth: 3, parent: bg} );  // absolute

     //   scene.create( { id: "CurveFG1", t: "path", d:"M110 180 Q150 190, 195 260", strokeColor: 0xFF0000ff, strokeWidth: 13, parent: bg} );  // absolute
//        scene.create( { id: "CurveFG1", t: "path", d:"M110 180 Q150 190, 195 260", strokeColor: 0x000000ff, strokeWidth: 3,  parent: bg} );  // absolute

//##################################################################################################################################
//
//------ CUBIC CURVE
//
//             var curve = scene.create( { t: "path", d:"M100 180 c100,100 250,100 250,200", strokeColor: 0x00FF88ff, strokeWidth: 3, parent: bg} );  // relative
//             var curve = scene.create( { t: "path", d:"M0 0 c100,100 250,100 250,200", strokeColor: 0x00FF88ff, strokeWidth: 3, parent: bg, x: 200, y: 400, w:10, h: 10 } );  // relative
//         scene.create( { t: "path", d:"M100,200 C100,100 250,100 250,200", strokeColor: 0x000000ff, strokeWidth: 7, parent: bg} );  // absolute
//         var curve = scene.create( { t: "path", d:"M100,200 C100,100 250,100 250,200 S400,300 400,200", strokeColor: 0x000000ff, strokeWidth: 7, parent: bg} );  // absolute
 
//         curve.ready.then
//         (
//          function(o)
//          {
//             curve.w = 310; curve.h = 210;
//
//             curve.animateTo({ x: 400.0  }, 10.75, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_FASTFORWARD, 1).then
//             (
//              function(o) {
//              
//              }
//              );
//         }
//        );
             
//             scene.create( { t: "path", d:"M0.330689 0 c -0.182634,0 -0.330689,0.148055 -0.330689,0.330689 0,0.182638 0.148055,0.330689 0.330689,0.330689 0.182634,0 0.330689,-0.148051 0.330689,-0.330689 0,-0.182634 -0.148055,-0.330689 -0.330689,-0.330689z", strokeColor: 0x000000ff, strokeWidth: 7, parent: bg} );
             
//          scene.create( { t: "path", d:"M89.171,155.172c2.668-0.742,4.23-3.531,3.475-6.238c-0.76-2.678-3.549-4.266-6.265-3.492 c-2.669,0.76-4.235,3.568-3.476,6.24C83.673,154.365,86.454,155.948,89.171,155.172z", strokeColor: 0x000000ff, strokeWidth: 7, parent: bg} );
     
//             scene.create( { t: "path", d:"rect( rx:'15', ry:'15' )", strokeColor: 0x00FF00ff, strokeWidth: 3, parent: bg, x: 600, y: 800, w: 300, h: 150;} );
//         scene.create( { t: "path", d:"M50 200 L350 200", strokeColor: 0x00FF00ff, strokeWidth: 3, parent: bg} );
             
//       scene.create( { t: "path", d:"M10 80 l100 0   0 100   -100 0   0 -100", fillColor: 0x00FF88ff, parent: bg} );
             
//       scene.create( { t: "path", d:"M10 80 Q50 10, 95 80 T 180 80 270 110", fillColor: 0x00FF88ff, parent: bg} );
//       scene.create( { t: "path", d:"M10 80 Q50 10, 95 80 T 180 80 270 110 M100 100 L200 200", fillColor: 0x00FF88ff, parent: bg} );
        
//##################################################################################################################################
        
// JUNK
//
//    if(false)
//    {
//      scene.create( { t: "path", d:"M300 600 l44.629 -10.01 42.188 -10.01 37.036 -8.618 35.498 -5.029 37.036 -1.392 42.188 0 55.664 0 60.791 -1.392 "
//                                 + "62.305 -4.98 60.059 -9.229 55.42 -10.01 35.498 -59.18 13.867 -59.229 8.716 -57.837 7.178 -54.199 8.716 -50.562 "
//                                 + "13.867 -49.17 20.41 -49.17 4.688 -10.01 -0.635 -8.691 -2.246 -5.029 -0.708 -1.392 4.443 0 17.92 0 23.071 -1.392 "
//                                 + "24.609 -4.98 23.071 -8.618 17.92 -10.01 15.479 -10.01 17.92 -16.113 42.236, -16.113Z",
//                      strokeColor: 0x000000ff,  strokeWidth: 7, strokeWidth: 3, fillColor: 0x00FF88ff, parent: bg} );
//    }
        
//        scene.create( { t: "path", d:"M 285.25596,665.33856 C 654.07555 712.27292 58.724179,415.02235 405.53052 458.53782",
//                     strokeColor: 0x000000ff,  x: e.x, y: e.y - 200, strokeWidth: 7,fillColor: 0x00FF88ff, parent: bg} );
    }
  });

  function updateSize(w,h)
  {
    bg.w = w;
    bg.h = h;
  }

  scene.on("onResize", function(e) { updateSize(e.w, e.h); });

  Promise.all([ bg ])
      .catch( (err) =>
      {
          console.log("SVG >> Loading Assets ... err = " + err);
      })
      .then( (success, failure) =>
      {
            console.log("\n START SVG >> Children of [bg] = " + bg.children.length);
            
        updateSize(scene.w, scene.h);
      });

}).catch( function importFailed(err){
  console.error("SVG >> Import failed for svgtest.js: " + err);
});