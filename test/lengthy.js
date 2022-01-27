const gamecontroller = require("../index");

gamecontroller.on("error", (data) => console.log("error", data));
gamecontroller.on("warning", (data) => console.log("warning", data));
gamecontroller.on("sdl-init", (data) => console.log("SDL2 Initialized", data));

// controller connected
gamecontroller.on("controller-device-added", (data) =>
  console.log("controller connected", data)
);

// Rumble (if supported) when A button is pressed
gamecontroller.on("a:down", (data) => {
  console.log(`player ${data.player} pressed A`);
  gamecontroller.rumble(60000, 40000, 100, data.player);
});

// Rumble triggers (if supported) when B button is pressed
gamecontroller.on("b:down", (data) => {
  console.log(`player ${data.player} pressed B`);
  gamecontroller.rumbleTriggers(40000, 40000, 100, data.player);
});

// Enable Gyroscope (if supported) when X button is pressed
gamecontroller.on("x:down", (data) => {
  console.log(`player ${data.player} pressed X`);
  gamecontroller.enableGyroscope(true);
});

// Disable Gyroscope (if supported) when X button is released
gamecontroller.on("x:up", (data) => {
  console.log(`player ${data.player} released X`);
  gamecontroller.enableGyroscope(false);
});

// Enable Accelerometer (if supported) when Y button is pressed
gamecontroller.on("y:down", (data) => {
  console.log(`player ${data.player} pressed Y`);
  gamecontroller.enableAccelerometer(true, data.player);
});

// Disable Accelerometer (if supported) when Y button is released
gamecontroller.on("y:up", (data) => {
  console.log(`player ${data.player} released Y`);
  gamecontroller.enableAccelerometer(false, data.player);
});

// Set LED colors (if supported) when leftstick button is pressed
var count = 0;
gamecontroller.on("leftstick:down", (data) => {
  console.log(`player ${data.player} pressed leftstick`);
  switch (count % 3) {
    case 0:
      gamecontroller.setLeds(0x0f, 0x62, 0xfe, data.player);
      break;
    case 1:
      gamecontroller.setLeds(0x19, 0x80, 0x38, data.player);
      break;
    case 2:
      gamecontroller.setLeds(0x8a, 0x3f, 0xfc, data.player);
      break;
  }
  count += 1;
});

// Axis motion
// gamecontroller.on("controller-axis-motion", (data) => {
//   console.log("axis motion", data);
// });

// Sensor updated
gamecontroller.on("controller-sensor-update", (data) =>
  console.log("sensor update", data)
);

gamecontroller.on("accelerometer:enabled", (data) =>
  console.log("accelerometer:enabled", data)
);

// Touchpad
gamecontroller.on("controller-touchpad-down", (data) =>
  console.log("controller-touchpad-down", data)
);

gamecontroller.on("controller-touchpad-up", (data) =>
  console.log("controller-touchpad-up", data)
);

gamecontroller.on("controller-touchpad-motion", (data) =>
  console.log("controller-touchpad-motion", data)
);

gamecontroller.on("led", (data) => console.log("LEDS set", data));

// Respond to both up & down events
gamecontroller.on("leftshoulder", (data) =>
  console.log(`player ${data.player} pressed:${data.pressed} leftshoulder`)
);

// Print information about a pressed button
gamecontroller.on("controller-button-down", (data) =>
  console.log("button pressed", data)
);

gamecontroller.on("back", (data) => process.exit(0));
