// /<reference path="custom.d.ts" />

const EventEmitter = require('events').EventEmitter;
const SdlGameController =
  require('bindings')('sdl_gamecontroller').SdlGameController;
const inherits = require('util').inherits;

inherits(SdlGameController, EventEmitter);

module.exports = (options = {}) => {
  const inst = new SdlGameController(options);
  let interval = options.interval || 33;
  if (options.fps) interval = 1000 / options.fps;
  setInterval(() => {
    inst.pollEvents();
  }, interval);
  return inst;
};
