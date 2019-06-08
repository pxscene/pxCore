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


// console.log("\n\n############### SAND BOX stuff - Only once !\n\n");

// These vars are populated/copied into the Contexified Context... aka Clone !
//

var vm            = require('vm');
var require       = this.require;

// JRJR warning... adding process global object to sandbox...
// Do we need to wrap... Some node apps are using hrtime
var _sandboxStuff = [ "console", "vm", "require", "process", "setTimeout", "clearTimeout", "setInterval", "clearInterval", "setImmediate", "clearImmediate" ];
