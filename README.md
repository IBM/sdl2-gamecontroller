# sdl2-gamecontroller

Game controller support for nodejs `EventEmitter` backed by [SDL2](https://www.libsdl.org/).
# Description

Do you want to use a game controller with your node app? You should first ask yourself “why” and you should examine your life choices that have brought you to this point.

If you do find you need to use a game controller with a node app, you are in the right place. I used SDL (Simple DirectMedia Layer) to power the controller because, in my experience, SDL just works. The most recent version has support for the DualSense, but it also has support for every other controller I have tried so far. The node addon is a thin wrapper around the SDL event loop and the SDL functions for rumble and lights.

[Article on LinkedIn](https://www.linkedin.com/pulse/how-use-sdl-powered-game-controller-nodejs-david-nixon)

# Installation

Install sdl2 and cmake. See [prerequisites](#Prerequisites)

```sh
yarn add sdl2-gamecontroller
```

```sh
npm install sdl2-gamecontroller
```
# Feedback

Found a bug or just what to let me know what you think. Open an issue on the [issues tab](https://github.com/IBM/sdl2-gamecontroller/issues) or [leave a comment](https://github.com/IBM/sdl2-gamecontroller/issues/9)

# Using the library

## Hello world

```js
const gamecontroller = require("sdl2-gamecontroller");

gamecontroller.on("error", (data) => console.log("error", data));
gamecontroller.on("warning", (data) => console.log("warning", data));
gamecontroller.on("sdl-init", () => console.log("SDL2 Initialized"));
gamecontroller.on("a:down", (data) => console.log("Hello A button world"));
```

## Lengthy example

```js
const gamecontroller = require("sdl2-gamecontroller");

gamecontroller.on("error", (data) => console.log("error", data));
gamecontroller.on("warning", (data) => console.log("warning", data));
gamecontroller.on("sdl-init", () => console.log("SDL2 Initialized"));

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
gamecontroller.on("controller-axis-motion", (data) =>
  console.log("axis motion", data)
);

// Sensor updated
gamecontroller.on("controller-sensor-update", (data) =>
  console.log("sensor update", data)
);

// Respond to both up & down events
gamecontroller.on("leftshoulder", (data) =>
  console.log(`player ${data.player} pressed:${data.pressed} leftshoulder`)
);

// Print information about a pressed button
gamecontroller.on("controller-button-down", (data) =>
  console.log("button pressed", data)
);
```

# API Docs

Full docs for the events and the functions calls
[API doc](docs/API.md)

# Prerequisites

NOTE: If you need rumble, LED, touchpad, gyroscope, or accelerometer support, you need SDL 2.0.16+ which seems not to be available form the Ubuntu/Red Hat repos.

## Linux **Red Hat**

```sh
sudo dnf install -y gcc-c++ cmake make
# You may need to enable the powertool repo
# dnf -y install dnf-plugins-core
# dnf config-manager --set-enabled powertools
sudo dnf install -y SDL2-devel
```

## Linux **Ubuntu**

```sh
sudo apt install -y build-essential cmake libsdl2-dev
```

## OSX

[install Homebrew](https://brew.sh/)

```sh
xcode-select --install
brew install cmake sdl2
```

## Linux - Latest SDL on Linux from Homebrew

[install Homebrew](https://brew.sh/)
install git `dnf install -y git` or `apt install git`

```sh
brew install sdl2
```

## Lastest SDL from source (OPTIONAL)

The version available from Homebrew is generally very recent but if you are dissatisfied with the version, you can [install from source](https://wiki.libsdl.org/Installation) with the instructions on the SDL2 wiki.

## Windows

I do not know if this works on Windows. If you get this working, please open a PR for changes to this doc and the code.
