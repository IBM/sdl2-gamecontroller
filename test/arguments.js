const gamecontroller = require('../index');

gamecontroller.on('error', (data) => console.log('error', data));
gamecontroller.on('warning', (data) => console.log('warning', data));
gamecontroller.on('sdl-init', (data) => console.log('SDL2 Initialized', data));
gamecontroller.on('a:down', () => console.log('Hello A button world'));
gamecontroller.on('controller-device-added', (data) => {
  console.log('controller connected', data.name);
  gamecontroller.rumble("bad",{bad:true}, false, undefined);
  gamecontroller.rumbleTriggers("bad",{bad:true}, false, undefined);
  gamecontroller.enableGyroscope(1, null);
  gamecontroller.enableAccelerometer(0, "one");
  gamecontroller.setLeds(undefined, false, "blue", "");
  process.exit(0);
});
