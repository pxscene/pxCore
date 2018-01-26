var isDuk = (typeof Duktape != "undefined")?true:false;

if (isDuk) {
global.console = require('console');
global.timers = require('timers');
global.Promise = require('bluebird');
global.process = require('process');

global.setTimeout = timers.setTimeout;
global.clearTimeout = timers.clearTimeout;
global.setInterval = timers.setInterval;
global.clearInterval = timers.clearInterval;

var schedule = function (fn) {
    var timer = uv.new_timer.call({});
    uv.timer_start(timer, 0, 0, fn);
};

Promise.setScheduler(schedule);
process.setScheduler(schedule);

global.constructPromise = function (obj) {
    return new Promise(function (resolve, reject) {
        // TODO Don't use the then method... reentrant recursion... 
        obj.then2(resolve, reject);
    });
}

global.constructProxy = function (obj) {
	print('constructProxy()');
    return new Proxy(obj, {
        has: function (targ, key) {
			print('proxy.has(' + key + ')');
            return _hasProxyFunc(targ, key);
        },
        get: function (targ, key, recv) {
			print('proxy.get(' + key + ')');
            var res = _getProxyFunc(targ, key);
            print('return = ' + res);
            if (res == undefined) {
                return targ[key];
            }
            return res;
        },
        set: function (targ, key, val, recv) {
			print('proxy.set(' + key + ')');
            var res = _setProxyFunc(targ, key, val);
            if (!res) {
                targ[key] = val;
            }
            return res;
        },
        deleteProperty: function (targ, key) {
			print('proxy.del(' + key + ')');
            _deleteProxyFunc(targ, key);
        }
    });
}

}

var AppSceneContext = require('rcvrcore/AppSceneContext');
var RPCController = require('rcvrcore/rpcController');

if (isDuk) {
global.loadUrl = function loadUrl(url) {

  var ctx = new AppSceneContext({        scene: getScene("scene.1"),
                                     makeReady: this.makeReady,
                                  getContextID: this.getContextID,
                                    packageUrl: url,
                                 rpcController: new RPCController() } );

  // console.log("JS >>>> loadURL()  ctx: " + getContextID() );

  ctx.loadScene();
}
}
else {
    function loadUrl(url) {

        var ctx = new AppSceneContext({        scene: getScene("scene.1"),
                                           makeReady: this.makeReady,
                                        getContextID: this.getContextID,
                                          packageUrl: url,
                                       rpcController: new RPCController() } );
      
        // console.log("JS >>>> loadURL()  ctx: " + getContextID() );
      
        ctx.loadScene();
      }
}

