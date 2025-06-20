// eslint-disable-next-line @typescript-eslint/no-require-imports, no-undef
const gamecontroller = require('sdl2-gamecontroller').default;

console.log('\n\n===== Commmon JS test');
gamecontroller.on('error', (data) => console.log('error', data));
gamecontroller.on('warning', (data) => console.log('warning', data));
gamecontroller.on('sdl-init', (data) => console.log('SDL2 Initialized', data));
gamecontroller.on('a:down', () => console.log('Hello A button world'));
gamecontroller.on('controller-device-added', (data) =>
  console.log('controller connected', data.name),
);
gamecontroller.on('x:down', () => process.exit(0));
