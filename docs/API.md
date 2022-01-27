# Events

- [error](#error)
- [warning](#warning)
- [sdl-init](#sdl-init)
- [controller-button-down](#controller-button-down)
- [controller-button-up](#controller-button-up)
- [controller-axis-motion](#controller-axis-motion)
- [controller-device-added](#controller-device-added)
- [controller-device-removed](#controller-device-removed)
- [controller-device-remapped](#controller-device-remapped)
- [controller-sensor-update](#controller-sensor-update)
- [accelerometer:enabled](#accelerometerenabled)
- [accelerometer:disabled](#accelerometerdisabled)
- [gyroscope:enabled](#gyroscopeenabled)
- [gyroscope:disabled](#gyroscopedisabled)
- [controller-touchpad-down](#controller-touchpad-down)
- [controller-touchpad-up](#controller-touchpad-up)
- [controller-touchpad-motion](#controller-touchpad-motion)
- [led](#led)
- [rumbled](#rumbled)
- [rumbled-triggers](#rumbled-triggers)

# Functions

- [enableGyroscope(enable, player)](#enableGyroscope)
- [enableAccelerometer(enable, player)](#enableAccelerometer)
- [setLeds(red, green, blue, player)](#setLeds)
- [rumble(low_frequency_rumble, high_frequency_rumble, duration_ms, player)](#rumble)
- [rumbleTriggers(left_rumble, right_rumble, duration_ms, player)](#rumbleTriggers)

## error

Emitted if something goes wrong. The `message` is set from [SDL_GetError](https://wiki.libsdl.org/SDL_GetError)

```js
{
  player: 2,
  message: 'That operation is not supported',
  operation: 'rumble'
}
```

## warning

Emitted if the event loop runs more than 100 ms. The event loop exits when this happens so some events will be delayed until the next polling.

```js
{
  message: 'Polling is taking too long.',
  elapsed_ms: 125,
  poll_number: 13
}
```

## sdl-init

The call to [SDL_Init](https://wiki.libsdl.org/SDL_Init) was successful.

```js
{
  compiled_against_SDL_version: '2.0.20',
  linkeded_against_SDL_version: '2.0.20'
}
```

## controller-button-down

Emitted when game controller button pressed.
[SDL_CONTROLLERBUTTONDOWN](https://wiki.libsdl.org/SDL_ControllerButtonEvent)

```js
{
  message: 'Game controller button released',
  button: 'a',
  pressed: true,
  player: 2 // SDL 2.0.12+
}
```

Two aliases for this event are also emitted with the event name set to the button and another event with the name set to name + ":down" name.
These names are defined in [SDL source code](https://github.com/libsdl-org/SDL/blob/release-2.0.16/src/joystick/SDL_gamecontroller.c)

- a
- b
- x
- y
- back
- guide
- start
- leftstick
- rightstick
- leftshoulder
- rightshoulder
- dpup
- dpdown
- dpleft
- dpright
- misc1
- paddle1
- paddle2
- paddle3
- paddle4
- touchpad

For example:

```js
gamecontroller.on("a:down", (data) => console.log("a pressed", data));
gamecontroller.on("b", (data) => console.log("b up or down", data));
```

## controller-button-up

Emitted when game controller button released.
[SDL_CONTROLLERBUTTONUP](https://wiki.libsdl.org/SDL_ControllerButtonEvent)

```js
{
  message: 'Game controller button released',
  button: 'a',
  pressed: true,
  player: 2 // SDL 2.0.12+
}
```

Two aliases for this event are also emitted with the event name set to the button and another event with the name set to name + ":up" name.
See [controller-button-down](#controller-button-down)

For example:

```js
gamecontroller.on("a:up", (data) => console.log("a pressed", data));
gamecontroller.on("b", (data) => console.log("b up or down", data));
```

## controller-axis-motion

Emitted for game controller axis motion.
[SDL_CONTROLLERAXISMOTION](https://wiki.libsdl.org/SDL_ControllerAxisEvent)

```js
{
  message: 'Game controller axis motion',
  button: 'righttrigger',
  timestamp: 5021,
  value: 7324,
  player: 1 // SDL 2.0.12+
}
```

An alias for this event is also emitted with the event name set to the axis name.
These names are defined in [SDL source code](https://github.com/libsdl-org/SDL/blob/release-2.0.16/src/joystick/SDL_gamecontroller.c)

- leftx
- lefty
- rightx
- righty
- lefttrigger
- righttrigger

For example:

```js
gamecontroller.on("leftx", (data) =>
  console.log("left stick moved in the x direction", data)
);
```

## controller-device-added

Emitted when a new game controller has been inserted into the system
[SDL_CONTROLLERDEVICEADDED](https://wiki.libsdl.org/SDL_ControllerDeviceEvent)

```js
{
  message: 'A new Game controller has been inserted into the system',
  which: 1,
  name: 'Nintendo Switch Pro Controller',
  vendor_id: 8406,
  product_id: 42769,
  player: 2,                    // SDL 2.0.12+
  serial_number: 'none',        // SDL 2.0.14+
  effects_supported: false,
  haptic: false,
  has_leds: 'false',            // SDL 2.0.14+
  num_touchpads: 0,             // SDL 2.0.14+
  has_accelerometer: false,     // SDL 2.0.14+
  has_gyroscope: false,         // SDL 2.0.14+
  has_rumble: false,            // SDL 2.0.10+
  has_rumble_trigger: false     // SDL 2.0.14+
}
```

## controller-device-removed

Emitted when an opened Game controller has been removed
[SDL_CONTROLLERDEVICEREMOVED](https://wiki.libsdl.org/SDL_ControllerDeviceEvent)

```js
{
  message: 'An opened Game controller has been removed',
  which: 1,
}
```

## controller-device-remapped

Emitted when the controller mapping is updated
[SDL_CONTROLLERDEVICEREMAPPED](https://wiki.libsdl.org/SDL_ControllerDeviceEvent)

```js
{
  message: 'The controller mapping was updated',
  which: 1,
}
```

## controller-sensor-update

Emitted when Game controller sensor is updated
[SDL_CONTROLLERSENSORUPDATE](https://wiki.libsdl.org/SDL_SensorGetData)

```js
// SDL 2.0.14+
{
  message: 'Game controller sensor was updated',
  sensor: 'gyroscope',
  x: 0.9674353003501892,
  y: 0.8535045385360718,
  z: 1.527079463005066
}
```

An alias for this event is also emitted with the event name set to either `gyroscope` or `accelerometer`.

For example:

```js
gamecontroller.on("gyroscope", (data) =>
  console.log("gyroscope updated", data)
);
```

## accelerometer:enabled

Emitted when the accelerometer is successfully enabled.
[SDL_GameControllerSetSensorEnabled](https://wiki.libsdl.org/SDL_GameControllerSetSensorEnabled)

```js
// SDL 2.0.14+
{
  player: 2;
}
```

## accelerometer:disabled

Emitted when the accelerometer is successfully disabled.
[SDL_GameControllerSetSensorEnabled](https://wiki.libsdl.org/SDL_GameControllerSetSensorEnabled)

```js
// SDL 2.0.14+
{
  player: 2;
}
```

## gyroscope:enabled

Emitted when the gyroscope is successfully enabled.
[SDL_GameControllerSetSensorEnabled](https://wiki.libsdl.org/SDL_GameControllerSetSensorEnabled)

```js
// SDL 2.0.14+
{
  player: 2;
}
```

## gyroscope:disabled

Emitted when the gyroscope is successfully disabled.
[SDL_GameControllerSetSensorEnabled](https://wiki.libsdl.org/SDL_GameControllerSetSensorEnabled)

```js
// SDL 2.0.14+
{
  player: 2;
}
```

## controller-touchpad-down

Emitted when Game controller touchpad was touched
[SDL_CONTROLLERTOUCHPADDOWN](https://wiki.libsdl.org/SDL_GameControllerGetTouchpadFinger)

```js
// SDL 2.0.14+
{
  message: 'Game controller touchpad was touched',
  touchpad: 0,
  finger: 0,
  x: 0.32760417461395264,
  y: 0.3672897219657898,
  pressure: 1
}
```

## controller-touchpad-up

Emitted when Game controller touchpad finger was lifted
[SDL_CONTROLLERTOUCHPADUP](https://wiki.libsdl.org/SDL_GameControllerGetTouchpadFinger)

```js
// SDL 2.0.14+
{
  message: 'Game controller touchpad finger was lifted',
  touchpad: 0,
  finger: 0,
  x: 0.22760418057441711,
  y: 0.40747663378715515,
  pressure: 0
}
```

## controller-touchpad-motion

Emitted when Game controller touchpad finger was moved
[SDL_CONTROLLERTOUCHPADMOTION](https://wiki.libsdl.org/SDL_GameControllerGetTouchpadFinger)

```js
// SDL 2.0.14+
{
  message: 'Game controller touchpad finger was moved',
  touchpad: 0,
  finger: 0,
  x: 0.6921875476837158,
  y: 0.3457944095134735,
  pressure: 1
}
```

## led

Emitted when the LEDs lights are successfully set.

```js
{
  player: 2;
}
```

## rumbled

Emitted when the controller is successfully rumbled.

```js
{
  player: 2;
}
```

## rumbled-triggers

Emitted when the controller triggers are successfully rumbled.

```js
{
  player: 2;
}
```

## enableGyroscope

`enableGyroscope(enable, player)`

- `enable` optional - defaults to true
- `player` optional - defaults to all players

  Example:

```js
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
```

## enableAccelerometer

`enableAccelerometer(enable, player)`

- `enable` optional - defaults to true
- `player` optional - defaults to all players

  Example:

```js
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
```

## setLeds

`setLeds(red, green, blue, player)`

- `red` optional - default to 0x00
- `green` optional - default to 0x00
- `blue` optional - default to 0xff
- `player` optional - defaults to all players

[SDL_GameControllerSetLED](https://wiki.libsdl.org/SDL_GameControllerSetLED)

Example:

```js
gamecontroller.on("leftstick:down", (data) => {
  gamecontroller.setLeds(0x0f, 0x62, 0xfe);
});
```

## rumble

`rumble(low_frequency_rumble, high_frequency_rumble, duration_ms, player)`

- `low_frequency_rumble` optional - defaults to 0xFFFC. Range seems to be 0x0200 - 0xFFFC
- `high_frequency_rumble` optional - defaults to 0xFFFC. Range seems to be 0x0200 - 0xFFFC
- `duration_ms` optional - defaults to 250.
- `player` optional - defaults to all players

[SDL_GameControllerRumble](https://wiki.libsdl.org/SDL_GameControllerRumble)

Example:

```js
// Rumble (if supported) when A button is pressed
gamecontroller.on("a:down", (data) => {
  gamecontroller.rumble(60000, 40000, 100, data.player);
});
```

## rumbleTriggers

`rumbleTriggers(left_rumble, right_rumble, duration_ms, player)`

- `left_rumble` optional - defaults to 0xFFFC. Range seems to be 0x0200 - 0xFFFC
- `right_rumble` optional - defaults to 0xFFFC. Range seems to be 0x0200 - 0xFFFC
- `duration_ms` optional - defaults to 250
- `player` optional - defaults to all players

[SDL_GameControllerRumbleTriggers](https://wiki.libsdl.org/SDL_GameControllerRumbleTriggers)

Example:

```js
// Rumble triggers (if supported) when B button is pressed
gamecontroller.on("b:down", (data) => {
  gamecontroller.rumbleTriggers(40000, 40000, 100, data.player);
});
```
