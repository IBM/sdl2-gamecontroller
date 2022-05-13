import customController from '../custom';

const gamecontroller = customController({
  sdl_joystick_rog_chakram: true,
  interval: 90,
});

gamecontroller.on('error', (data) => console.log('error', data));
gamecontroller.on('warning', (data) => console.log('warning', data));
gamecontroller.on('sdl-init', (data) => console.log('SDL2 Initialized', data));
gamecontroller.on('a:down', () => console.log('Hello A button world'));
gamecontroller.on('controller-device-added', (data) =>
  console.log('controller connected', data.name),
);
gamecontroller.on('x:down', () => process.exit(0));
