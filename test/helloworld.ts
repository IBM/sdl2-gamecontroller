import gamecontroller from '../index';

gamecontroller.on('error', (data) => console.log('error', data));
gamecontroller.on('warning', (data) => console.log('warning', data));
gamecontroller.on('sdl-init', (data) => console.log('SDL2 Initialized', data));
gamecontroller.on('a:down', (data) => console.log('Hello A button world'));
gamecontroller.on('controller-device-added', (data) =>
  console.log('controller connected', data.name),
);
gamecontroller.on('x:down', (data) => process.exit(0));
