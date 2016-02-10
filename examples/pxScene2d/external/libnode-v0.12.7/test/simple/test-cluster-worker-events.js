// Copyright Joyent, Inc. and other Node contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit
// persons to whom the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
// NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
// USE OR OTHER DEALINGS IN THE SOFTWARE.


var assert = require('assert');
var cluster = require('cluster');

var OK = 2;

if (cluster.isMaster) {

  var worker = cluster.fork();

  worker.on('exit', function(code) {
    assert.equal(code, OK);
    process.exit(0);
  });

  worker.send('SOME MESSAGE');

  return;
}

// Messages sent to a worker will be emitted on both the process object and the
// process.worker object.

assert(cluster.isWorker);

var sawProcess;
var sawWorker;

process.on('message', function(m) {
  assert(!sawProcess);
  sawProcess = true;
  check(m);
});

cluster.worker.on('message', function(m) {
  assert(!sawWorker);
  sawWorker = true;
  check(m);
});

var messages = [];

function check(m) {
  messages.push(m);

  if (messages.length < 2) return;

  assert.deepEqual(messages[0], messages[1]);

  cluster.worker.once('error', function(e) {
    assert.equal(e, 'HI');
    process.exit(OK);
  });

  process.emit('error', 'HI');
}
