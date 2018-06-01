"use strict";

function setTimeout(cb, delay, a1, a2, a3) {
    var tid = uv.new_timer.call();
    uv.timer_start(tid, delay, 0, function () { uv.close(tid); cb(a1, a2, a3, tid); });
    return tid;
}

function clearTimeout(tid) {
    uv.timer_stop(tid);
}

function closeTimer(tid) {
    uv.close(tid);
}

function setInterval(cb, delay, a1, a2, a3) {
    var tid = uv.new_timer.call();
    uv.timer_start(tid, delay, delay, function () { cb(a1, a2, a3); });
    return tid;
}

function clearInterval(tid) {
    uv.timer_stop(tid);
}

module.exports = {
    setTimeout: setTimeout,
    clearTimeout: clearTimeout,
    setInterval: setInterval,
    clearInterval: clearInterval,
    closeTimer: closeTimer
}
