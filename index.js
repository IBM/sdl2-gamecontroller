const EventEmitter = require("events").EventEmitter;
const SdlGameController =
  require("bindings")("sdl_gamecontroller").SdlGameController;
const inherits = require("util").inherits;

inherits(SdlGameController, EventEmitter);

const gamecontroller = new SdlGameController();
setInterval(() => {
  gamecontroller.pollEvents();
}, 33);

module.exports = gamecontroller;
