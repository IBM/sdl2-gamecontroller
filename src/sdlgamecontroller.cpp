#include "sdlgamecontroller.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_gamecontroller.h>
#include <chrono>
#include <set>
#include <string>

bool SdlGameController::sdlInit = false;

Napi::FunctionReference SdlGameController::constructor;

Napi::Object SdlGameController::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func =
    DefineClass(env, "SdlGameController",
                {InstanceMethod("pollEvents", &SdlGameController::pollEvents),
                 InstanceMethod("enableGyroscope",
                                &SdlGameController::enableGyroscope),
                 InstanceMethod("enableAccelerometer",
                                &SdlGameController::enableAccelerometer),
                 InstanceMethod("setLeds", &SdlGameController::setLeds),
                 InstanceMethod("rumble", &SdlGameController::rumble),
                 InstanceMethod("rumbleTriggers",
                                &SdlGameController::rumbleTriggers)});

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("SdlGameController", func);

  return exports;
}

SdlGameController::SdlGameController(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<SdlGameController>(info), poll_number(0) {
  if (info.Length() > 0) {
    Napi::Object config = info[0].As<Napi::Object>();
    Napi::Value value = config.Get("sdl_joystick_rog_chakram");
    auto sdl_joystick_rog_chakram = value.ToBoolean();
    if (sdl_joystick_rog_chakram)
      this->hints.insert("sdl_joystick_rog_chakram");
  }
}

/* PS5 trigger effect documentation:
   https://controllers.fandom.com/wiki/Sony_DualSense#FFB_Trigger_Modes
*/
typedef struct {
  Uint8 ucEnableBits1;              /* 0 */
  Uint8 ucEnableBits2;              /* 1 */
  Uint8 ucRumbleRight;              /* 2 */
  Uint8 ucRumbleLeft;               /* 3 */
  Uint8 ucHeadphoneVolume;          /* 4 */
  Uint8 ucSpeakerVolume;            /* 5 */
  Uint8 ucMicrophoneVolume;         /* 6 */
  Uint8 ucAudioEnableBits;          /* 7 */
  Uint8 ucMicLightMode;             /* 8 */
  Uint8 ucAudioMuteBits;            /* 9 */
  Uint8 rgucRightTriggerEffect[11]; /* 10 */
  Uint8 rgucLeftTriggerEffect[11];  /* 21 */
  Uint8 rgucUnknown1[6];            /* 32 */
  Uint8 ucLedFlags;                 /* 38 */
  Uint8 rgucUnknown2[2];            /* 39 */
  Uint8 ucLedAnim;                  /* 41 */
  Uint8 ucLedBrightness;            /* 42 */
  Uint8 ucPadLights;                /* 43 */
  Uint8 ucLedRed;                   /* 44 */
  Uint8 ucLedGreen;                 /* 45 */
  Uint8 ucLedBlue;                  /* 46 */
} DS5EffectsState_t;

bool SdlGameController::SendEffect(SDL_GameController *gamecontroller) {
#if SDL_VERSION_ATLEAST(2, 0, 14)
  if (SDL_GameControllerGetType(gamecontroller) != SDL_CONTROLLER_TYPE_PS5)
    return SDL_FALSE;

  Uint8 clear[11] = {0x05, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  DS5EffectsState_t state;
  SDL_zero(state);
  state.ucEnableBits1 |=
    (0x04 | 0x08); /* Modify right and left trigger effect respectively */
  SDL_memcpy(state.rgucRightTriggerEffect, clear, sizeof(clear));
  SDL_memcpy(state.rgucLeftTriggerEffect, clear, sizeof(clear));
  auto success =
    SDL_GameControllerSendEffect(gamecontroller, &state, sizeof(state)) == 0;
  return success;
#else
  (void) gamecontroller;
  return SDL_FALSE;
#endif
}

SDL_GameController *SdlGameController::AddController(const int device_index,
                                                     Napi::Object *obj) {
  SDL_JoystickID controller_id = SDL_JoystickGetDeviceInstanceID(device_index);
  if (controller_id < 0) {
    SDL_Log("Couldn't get controller ID: %s\n", SDL_GetError());
    return nullptr;
  }

  auto controller = SdlGameController::gamecontrollers[controller_id];
  if (controller) {
    // We already have this controller
    return controller;
  }

  // remember this controller
  controller = SDL_GameControllerOpen(device_index);
  if (controller)
    gamecontrollers[controller_id] = controller;
  else
    return nullptr;

  obj->Set("message",
           "A new Game controller has been inserted into the system");

  obj->Set("name", SDL_GameControllerName(controller));
  obj->Set("which", device_index);
  obj->Set("name", SDL_GameControllerName(controller));
  obj->Set("vendor_id", SDL_GameControllerGetVendor(controller));
  obj->Set("product_id", SDL_GameControllerGetProduct(controller));

#if SDL_VERSION_ATLEAST(2, 0, 18)
  auto trigger = SDL_GameControllerHasRumbleTriggers(controller) ? true : false;
  obj->Set("has_rumble_trigger", trigger);
#endif

#if SDL_VERSION_ATLEAST(2, 0, 14)
  auto sn = SDL_GameControllerGetSerial(controller);
  if (sn)
    obj->Set("serial_number", sn);
  else
    obj->Set("serial_number", "none");

  auto hasLeds = SDL_GameControllerHasLED(controller) ? true : false;
  obj->Set("has_leds", hasLeds);

  auto nTouchPads = SDL_GameControllerGetNumTouchpads(controller);
  obj->Set("num_touchpads", nTouchPads);

  auto accelerometer =
    SDL_GameControllerHasSensor(controller, SDL_SENSOR_ACCEL) ? true : false;
  obj->Set("has_accelerometer", accelerometer);

  auto gyroscope =
    SDL_GameControllerHasSensor(controller, SDL_SENSOR_GYRO) ? true : false;
  obj->Set("has_gyroscope", gyroscope);
#endif

#if SDL_VERSION_ATLEAST(2, 0, 12)
  auto player = NextPlayer();
  SDL_GameControllerSetPlayerIndex(controller, player);
  obj->Set("player", player);
#endif

  auto effects = SendEffect(controller) ? true : false;
  obj->Set("effects_supported", effects);

  auto js = SDL_GameControllerGetJoystick(controller);
  auto isHaptic = SDL_JoystickIsHaptic(js) ? true : false;
  obj->Set("haptic", isHaptic);

  SDL_ClearError();
  // Range seems to be 0x0200 - 0xFFFC
#if SDL_VERSION_ATLEAST(2, 0, 10)
  auto rumble = SDL_GameControllerRumble(controller, 0x0200, 0x0200, 250) >= 0;
  obj->Set("has_rumble", rumble);
#endif

  return controller;
}

void SdlGameController::RemoveController(const SDL_JoystickID which) {
  auto search = gamecontrollers.find(which);
  if (search != gamecontrollers.end()) {
    gamecontrollers.erase(search);
  }
}

int SdlGameController::NextPlayer() {
#if SDL_VERSION_ATLEAST(2, 0, 12)
  // find current players
  std::set<int> players;
  for (auto &controller : gamecontrollers) {
    auto player = SDL_GameControllerGetPlayerIndex(controller.second);
    players.insert(player);
  }

  // Find the first available player slot. Max number of players is 8 probably.
  for (int player = 1; player < 8; player++) {
    auto search = players.find(player);
    if (search == players.end()) {
      return player;
    }
  }
#endif
  return -1;
}

Napi::Value SdlGameController::pollEvents(const Napi::CallbackInfo &info) {
  // do not spend too long here
  auto start = std::chrono::system_clock::now();
  this->poll_number++;

  Napi::Env env = info.Env();
  Napi::Function emit_unbound =
    info.This().As<Napi::Object>().Get("emit").As<Napi::Function>();
  Napi::Function emit = emit_unbound.Get("bind")
                          .As<Napi::Function>()
                          .Call(emit_unbound, {info.This()})
                          .As<Napi::Function>();

  // Set up SDL
  if (!SdlGameController::sdlInit) {
    SDL_SetHint(SDL_HINT_ACCELEROMETER_AS_JOYSTICK, "0");
#if SDL_VERSION_ATLEAST(2, 0, 16)
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_JOY_CONS, "1");
#endif
#if SDL_VERSION_ATLEAST(2, 0, 10)
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS4_RUMBLE, "1");
#endif
#if SDL_VERSION_ATLEAST(2, 0, 16)
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS5_RUMBLE, "1");
#endif
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
#if SDL_VERSION_ATLEAST(2, 0, 22)
    if (this->hints.count("sdl_joystick_rog_chakram") > 0) {
      SDL_SetHint(SDL_HINT_JOYSTICK_ROG_CHAKRAM, "1");
    }
#endif
#if SDL_VERSION_ATLEAST(2, 0, 24)
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_SHIELD, "1");
#endif
#if SDL_VERSION_ATLEAST(2, 0, 26)
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_XBOX_360, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_XBOX_360_PLAYER_LED, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_XBOX_360_WIRELESS, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_XBOX_ONE, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_XBOX_ONE_HOME_LED, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_WII_PLAYER_LED, "1");
#endif

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER)
        < 0) {
      emit({Napi::String::New(env, "error"),
            Napi::String::New(env, SDL_GetError())});
    } else {
      SDL_version compiled;
      SDL_version linked;

      SDL_VERSION(&compiled);
      SDL_GetVersion(&linked);

      auto info = Napi::Object::New(env);
      std::string compile_info = std::to_string(compiled.major) + "."
                                 + std::to_string(compiled.minor) + "."
                                 + std::to_string(compiled.patch);
      info.Set("compiled_against_SDL_version", compile_info);
      std::string link_info = std::to_string(linked.major) + "."
                              + std::to_string(linked.minor) + "."
                              + std::to_string(linked.patch);
      info.Set("linked_against_SDL_version", link_info);
#if SDL_VERSION_ATLEAST(2, 0, 22)
      if (this->hints.count("sdl_joystick_rog_chakram") > 0) {
        info.Set("using_hints", "sdl_joystick_rog_chakram");
      }
#endif
      emit({Napi::String::New(env, "sdl-init"), info});
      SdlGameController::sdlInit = true;

      for (auto i = 0; i < SDL_NumJoysticks(); ++i) {
        if (SDL_IsGameController(i)) {
          auto obj = Napi::Object::New(env);
          auto gc = AddController(i, &obj);
          if (gc) {
            obj.Set("operation", "SDL_Init");
            emit({Napi::String::New(env, "controller-device-added"), obj});
          } else {
            obj.Set("message", SDL_GetError());
            obj.Set("operation", "SDL_GameControllerOpen");
            emit({Napi::String::New(env, "error"), obj});
          }
        }
      }
    }
  }

  // Ignore non-game controller events -- Does this help performance?
  // SDL_FlushEvents(SDL_APP_TERMINATING, SDL_CONTROLLERAXISMOTION - 1);
  // SDL_FlushEvents(SDL_DOLLARGESTURE, SDL_SENSORUPDATE - 1);
  // SDL_FlushEvents(SDL_RENDER_TARGETS_RESET, SDL_LASTEVENT);

  // poll until all events are handled!
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    auto obj = Napi::Object::New(env);

    // only collect events for a "while". If we take too long just quit.
    auto now = std::chrono::system_clock::now();
    auto msSofar =
      std::chrono::duration_cast<std::chrono::milliseconds>(now - start)
        .count();
    if (poll_number > 1 && msSofar > 100) {
      obj.Set("message", "Polling is taking too long.");
      obj.Set("elapsed_ms", msSofar);
      obj.Set("poll_number", poll_number);
      emit({Napi::String::New(env, "warning"), obj});
      break;
    }

    SDL_GameController *gc;
    std::string gcBtn;

    switch (event.type) {
      case SDL_CONTROLLERDEVICEADDED:
        gc = AddController(event.cdevice.which, &obj);
        if (gc) {
          auto msg = obj.Get("message");
          // do not emit the message if the controller was previously found
          if (msg.IsString()) {
            obj.Set("operation", "SDL_PollEvent");
            emit({Napi::String::New(env, "controller-device-added"), obj});
          }
        } else {
          obj.Set("message", SDL_GetError());
          obj.Set("operation", "SDL_GameControllerOpen");
          emit({Napi::String::New(env, "error"), obj});
        }
        break;
      case SDL_CONTROLLERDEVICEREMOVED:
        obj.Set("message", "An opened Game controller has been removed");
        obj.Set("which", static_cast<int>(event.cdevice.which));
        RemoveController(static_cast<int>(event.cdevice.which));
        emit({Napi::String::New(env, "controller-device-removed"), obj});
        break;

      case SDL_CONTROLLERAXISMOTION:
        obj.Set("message", "Game controller axis motion");
        gcBtn = SDL_GameControllerGetStringForAxis(
          static_cast<SDL_GameControllerAxis>(event.jaxis.axis));
        obj.Set("button", gcBtn);
        obj.Set("timestamp", event.jaxis.timestamp);
        obj.Set("value", event.jaxis.value);

#if SDL_VERSION_ATLEAST(2, 0, 12)
        gc = gamecontrollers[event.jaxis.which];
        if (gc) {
          auto player = SDL_GameControllerGetPlayerIndex(gc);
          obj.Set("player", player);
        }
#endif

        emit({Napi::String::New(env, gcBtn), obj});
        emit({Napi::String::New(env, "controller-axis-motion"), obj});
        break;
      case SDL_CONTROLLERBUTTONDOWN:
        obj.Set("message", "Game controller button pressed");
        gcBtn = SDL_GameControllerGetStringForButton(
          static_cast<SDL_GameControllerButton>(event.cbutton.button));
        obj.Set("button", gcBtn);
        obj.Set("pressed", true);

#if SDL_VERSION_ATLEAST(2, 0, 12)
        gc = gamecontrollers[event.cbutton.which];
        if (gc) {
          auto player = SDL_GameControllerGetPlayerIndex(gc);
          obj.Set("player", player);
        }
#endif

        emit({Napi::String::New(env, gcBtn + ":down"), obj});
        emit({Napi::String::New(env, gcBtn), obj});
        emit({Napi::String::New(env, "controller-button-down"), obj});
        break;
      case SDL_CONTROLLERBUTTONUP:
        obj.Set("message", "Game controller button released");
        gcBtn = SDL_GameControllerGetStringForButton(
          static_cast<SDL_GameControllerButton>(event.cbutton.button));
        obj.Set("button", gcBtn);
        obj.Set("pressed", false);

#if SDL_VERSION_ATLEAST(2, 0, 12)
        gc = gamecontrollers[event.cbutton.which];
        if (gc) {
          auto player = SDL_GameControllerGetPlayerIndex(gc);
          obj.Set("player", player);
        }
#endif

        emit({Napi::String::New(env, gcBtn + ":up"), obj});
        emit({Napi::String::New(env, gcBtn), obj});
        emit({Napi::String::New(env, "controller-button-up"), obj});
        break;

      case SDL_CONTROLLERDEVICEREMAPPED:
        obj.Set("message", "The controller mapping was updated");
        obj.Set("which", static_cast<int>(event.cdevice.which));
        emit({Napi::String::New(env, "controller-device-remapped"), obj});
        break;

#if SDL_VERSION_ATLEAST(2, 0, 14)
      case SDL_CONTROLLERTOUCHPADDOWN:
        obj.Set("message", "Game controller touchpad was touched");
        obj.Set("touchpad", event.ctouchpad.touchpad);
        obj.Set("finger", event.ctouchpad.finger);
        obj.Set("x", event.ctouchpad.x);
        obj.Set("y", event.ctouchpad.y);
        obj.Set("pressure", event.ctouchpad.pressure);
        emit({Napi::String::New(env, "controller-touchpad-down"), obj});
        break;
      case SDL_CONTROLLERTOUCHPADMOTION:
        obj.Set("message", "Game controller touchpad finger was moved");
        obj.Set("touchpad", event.ctouchpad.touchpad);
        obj.Set("finger", event.ctouchpad.finger);
        obj.Set("x", event.ctouchpad.x);
        obj.Set("y", event.ctouchpad.y);
        obj.Set("pressure", event.ctouchpad.pressure);
        emit({Napi::String::New(env, "controller-touchpad-motion"), obj});
        break;
      case SDL_CONTROLLERTOUCHPADUP:
        obj.Set("message", "Game controller touchpad finger was lifted");
        obj.Set("touchpad", event.ctouchpad.touchpad);
        obj.Set("finger", event.ctouchpad.finger);
        obj.Set("x", event.ctouchpad.x);
        obj.Set("y", event.ctouchpad.y);
        obj.Set("pressure", event.ctouchpad.pressure);
        emit({Napi::String::New(env, "controller-touchpad-up"), obj});
        break;

      case SDL_CONTROLLERSENSORUPDATE:
        obj.Set("message", "Game controller sensor was updated");
        switch (event.csensor.sensor) {
          case SDL_SENSOR_GYRO:
            obj.Set("sensor", "gyroscope");
            obj.Set("x", event.csensor.data[0]);
            obj.Set("y", event.csensor.data[1]);
            obj.Set("z", event.csensor.data[2]);
            emit({Napi::String::New(env, "gyroscope"), obj});
            break;
          case SDL_SENSOR_ACCEL:
            obj.Set("sensor", "accelerometer");
            obj.Set("x", event.csensor.data[0]);
            obj.Set("y", event.csensor.data[1]);
            obj.Set("z", event.csensor.data[2]);
            emit({Napi::String::New(env, "accelerometer"), obj});
            break;
          default:
            obj.Set("sensor", "unknown");
            obj.Set("x", event.csensor.data[0]);
            obj.Set("y", event.csensor.data[1]);
            obj.Set("z", event.csensor.data[2]);
            break;
        }
        emit({Napi::String::New(env, "controller-sensor-update"), obj});
        break;
#endif
#if SDL_VERSION_ATLEAST(2, 0, 24)
      case SDL_JOYBATTERYUPDATED:
        obj.Set("message", "Game controller battery was updated");
        obj.Set("timestamp", event.jbattery.timestamp);
        obj.Set("which", static_cast<int>(event.jbattery.which));
        switch (event.jbattery.level) {
          case SDL_JOYSTICK_POWER_EMPTY:
            obj.Set("level", "empty");
            break;
          case SDL_JOYSTICK_POWER_LOW:
            obj.Set("level", "low");
            break;
          case SDL_JOYSTICK_POWER_MEDIUM:
            obj.Set("level", "medium");
            break;
          case SDL_JOYSTICK_POWER_FULL:
            obj.Set("level", "full");
            break;
          case SDL_JOYSTICK_POWER_WIRED:
            obj.Set("level", "wired");
            break;
          case SDL_JOYSTICK_POWER_MAX:
            obj.Set("level", "max");
            break;
          default:
            obj.Set("level", "unknown");
        }
        emit({Napi::String::New(env, "controller-battery-update"), obj});
        break;
#endif
        // LIMITED support for keyboard events - probably only helpful for
        // testing
      case SDL_KEYDOWN:
      case SDL_KEYUP:
        if (event.key.keysym.scancode == SDL_SCANCODE_A
            || event.key.keysym.scancode == SDL_SCANCODE_B
            || event.key.keysym.scancode == SDL_SCANCODE_X
            || event.key.keysym.scancode == SDL_SCANCODE_Y) {
          if (event.key.state == SDL_PRESSED) {
            obj.Set("message", "Game controller button pressed");
            gcBtn =
              SDL_GetKeyName(SDL_GetKeyFromScancode(event.key.keysym.scancode));
            gcBtn[0] = std::tolower(gcBtn[0]);
            obj.Set("button", gcBtn);
            obj.Set("pressed", true);
            emit({Napi::String::New(env, gcBtn + ":down"), obj});
            emit({Napi::String::New(env, gcBtn), obj});
            emit({Napi::String::New(env, "controller-button-down"), obj});
          } else {
            obj.Set("message", "Game controller button released");
            gcBtn =
              SDL_GetKeyName(SDL_GetKeyFromScancode(event.key.keysym.scancode));
            gcBtn[0] = std::tolower(gcBtn[0]);
            obj.Set("button", gcBtn);
            obj.Set("pressed", false);
            emit({Napi::String::New(env, gcBtn + ":up"), obj});
            emit({Napi::String::New(env, gcBtn), obj});
            emit({Napi::String::New(env, "controller-button-up"), obj});
          }
        }
    }
  }

  return Napi::String::New(env, "OK");
}

void SdlGameController::enableGyroscope(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::Function emit_unbound =
    info.This().As<Napi::Object>().Get("emit").As<Napi::Function>();
  Napi::Function emit = emit_unbound.Get("bind")
                          .As<Napi::Function>()
                          .Call(emit_unbound, {info.This()})
                          .As<Napi::Function>();

  SDL_bool enable = SDL_TRUE;
  int playerNumber = 0;  // enable for all players
  auto warning = Napi::Object::New(env);

  // Check if enable is set.
  if (info.Length() > 0) {
    if (info[0].IsBoolean()) {
      enable = info[0].ToBoolean() ? SDL_TRUE : SDL_FALSE;
    } else {
      warning.Set("message", "wrong argument type: enable");
      emit({Napi::String::New(env, "warning"), warning});
    }
  }

  // Check if player number is set
  if (info.Length() > 1) {
    if (info[0].IsNumber()) {
      playerNumber = info[1].ToNumber();
    } else {
      warning.Set("message", "wrong argument type: player");
      emit({Napi::String::New(env, "warning"), warning});
    }
  }

  for (auto &controller : gamecontrollers) {
    auto obj = Napi::Object::New(env);
#if SDL_VERSION_ATLEAST(2, 0, 12)
    auto player = SDL_GameControllerGetPlayerIndex(controller.second);
    obj.Set("player", player);
#else
    auto player = playerNumber;
#endif
    if (playerNumber == 0 || playerNumber == player) {
#if SDL_VERSION_ATLEAST(2, 0, 14)
      auto success = SDL_GameControllerSetSensorEnabled(controller.second,
                                                        SDL_SENSOR_GYRO, enable)
                     == 0;
#else
      (void) controller;
      auto success = false;
#endif
      if (success) {
        if (enable)
          emit({Napi::String::New(env, "gyroscope:enabled"), obj});
        else
          emit({Napi::String::New(env, "gyroscope:disabled"), obj});
      } else {
        obj.Set("message", SDL_GetError());
        obj.Set("operation", "enableGyroscope");
        emit({Napi::String::New(env, "error"), obj});
      }
    }
  }
}

void SdlGameController::enableAccelerometer(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::Function emit_unbound =
    info.This().As<Napi::Object>().Get("emit").As<Napi::Function>();
  Napi::Function emit = emit_unbound.Get("bind")
                          .As<Napi::Function>()
                          .Call(emit_unbound, {info.This()})
                          .As<Napi::Function>();

  int playerNumber = 0;  // enable for all players
  SDL_bool enable = SDL_TRUE;
  auto warning = Napi::Object::New(env);

  // Check the number of arguments passed.
  if (info.Length() > 0) {
    if (info[0].IsBoolean()) {
      enable = info[0].ToBoolean() ? SDL_TRUE : SDL_FALSE;
    } else {
      warning.Set("message", "wrong argument type: enable");
      emit({Napi::String::New(env, "warning"), warning});
    }
  }

  if (info.Length() > 1) {
    if (info[1].IsNumber()) {
      playerNumber = info[1].ToNumber();
    } else {
      warning.Set("message", "wrong argument type: player");
      emit({Napi::String::New(env, "warning"), warning});
    }
  }

  for (auto &controller : gamecontrollers) {
#if SDL_VERSION_ATLEAST(2, 0, 12)
    auto player = SDL_GameControllerGetPlayerIndex(controller.second);
#else
    auto player = playerNumber;
#endif
    if (playerNumber == 0 || playerNumber == player) {
      auto obj = Napi::Object::New(env);
      obj.Set("player", player);
#if SDL_VERSION_ATLEAST(2, 0, 14)
      auto success =
        SDL_GameControllerSetSensorEnabled(controller.second, SDL_SENSOR_ACCEL,
                                           enable)
        == 0;
#else
      (void) controller;
      auto success = false;
#endif
      if (success) {
        if (enable)
          emit({Napi::String::New(env, "accelerometer:enabled"), obj});
        else
          emit({Napi::String::New(env, "accelerometer:disabled"), obj});
      } else {
        obj.Set("message", SDL_GetError());
        obj.Set("operation", "enableAccelerometer");
        emit({Napi::String::New(env, "error"), obj});
      }
    }
  }
}

void SdlGameController::rumble(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::Function emit_unbound =
    info.This().As<Napi::Object>().Get("emit").As<Napi::Function>();
  Napi::Function emit = emit_unbound.Get("bind")
                          .As<Napi::Function>()
                          .Call(emit_unbound, {info.This()})
                          .As<Napi::Function>();

  int playerNumber = 0;  // rumble all players
  Uint16 low_frequency_rumble = 0xFFFC;
  Uint16 high_frequency_rumble = 0xFFFC;
  Uint32 duration_ms = 250;
  auto warning = Napi::Object::New(env);

  // Check for low frequency
  if (info.Length() > 0) {
    if (info[0].IsNumber()) {
      low_frequency_rumble = static_cast<uint32_t>(info[0].ToNumber());
    } else {
      warning.Set("message", "wrong argument type: low_frequency_rumble");
      emit({Napi::String::New(env, "warning"), warning});
    }
  }

  // Check for high frequency
  if (info.Length() > 1) {
    if (info[1].IsNumber()) {
      high_frequency_rumble = static_cast<uint32_t>(info[1].ToNumber());
    } else {
      warning.Set("message", "wrong argument type: high_frequency_rumble");
      emit({Napi::String::New(env, "warning"), warning});
    }
  }

  // Check for high frequency
  if (info.Length() > 2) {
    if (info[2].IsNumber()) {
      duration_ms = static_cast<uint32_t>(info[2].ToNumber());
    } else {
      warning.Set("message", "wrong argument type: duration_ms");
      emit({Napi::String::New(env, "warning"), warning});
    }
  }

  // Check for player number
  if (info.Length() > 3) {
    if (info[3].IsNumber()) {
      playerNumber = info[3].ToNumber();
    } else {
      warning.Set("message", "wrong argument type: player");
      emit({Napi::String::New(env, "warning"), warning});
    }
  }

  for (auto &controller : gamecontrollers) {
#if SDL_VERSION_ATLEAST(2, 0, 12)
    auto player = SDL_GameControllerGetPlayerIndex(controller.second);
#else
    auto player = playerNumber;
#endif
    if (playerNumber == 0 || playerNumber == player) {
      auto obj = Napi::Object::New(env);
      obj.Set("player", player);
#if SDL_VERSION_ATLEAST(2, 0, 10)
      auto success =
        SDL_GameControllerRumble(controller.second, low_frequency_rumble,
                                 high_frequency_rumble, duration_ms);
#else
      (void) controller;
      (void) low_frequency_rumble;
      (void) high_frequency_rumble;
      (void) duration_ms;
      int success = -1;
#endif
      if (success >= 0) {
        emit({Napi::String::New(env, "rumbled"), obj});
      } else {
        obj.Set("message", SDL_GetError());
        obj.Set("operation", "rumble");
        emit({Napi::String::New(env, "error"), obj});
      }
    }
  }
}

void SdlGameController::rumbleTriggers(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::Function emit_unbound =
    info.This().As<Napi::Object>().Get("emit").As<Napi::Function>();
  Napi::Function emit = emit_unbound.Get("bind")
                          .As<Napi::Function>()
                          .Call(emit_unbound, {info.This()})
                          .As<Napi::Function>();

  int playerNumber = 0;  // rumble all players
  Uint16 left_rumble = 0xFFFC;
  Uint16 right_rumble = 0xFFFC;
  Uint32 duration_ms = 250;
  auto warning = Napi::Object::New(env);

  // Check for left
  if (info.Length() > 0) {
    if (info[0].IsNumber()) {
      left_rumble = static_cast<uint32_t>(info[0].ToNumber());
    } else {
      warning.Set("message", "wrong argument type: left_rumble");
      emit({Napi::String::New(env, "warning"), warning});
    }
  }

  // Check for right
  if (info.Length() > 1) {
    if (info[1].IsNumber()) {
      right_rumble = static_cast<uint32_t>(info[1].ToNumber());
    } else {
      warning.Set("message", "wrong argument type: right_rumble");
      emit({Napi::String::New(env, "warning"), warning});
    }
  }

  // Check for duration
  if (info.Length() > 2) {
    if (info[2].IsNumber()) {
      duration_ms = static_cast<uint32_t>(info[2].ToNumber());
    } else {
      warning.Set("message", "wrong argument type: duration_ms");
      emit({Napi::String::New(env, "warning"), warning});
    }
  }

  // Check for player number
  if (info.Length() > 3) {
    // Check the argument types
    if (info[3].IsNumber()) {
      playerNumber = info[3].ToNumber();
    } else {
      warning.Set("message", "wrong argument type: player");
      emit({Napi::String::New(env, "warning"), warning});
    }
  }

  for (auto &controller : gamecontrollers) {
#if SDL_VERSION_ATLEAST(2, 0, 12)
    auto player = SDL_GameControllerGetPlayerIndex(controller.second);
#else
    auto player = playerNumber;
#endif
    if (playerNumber == 0 || playerNumber == player) {
      auto obj = Napi::Object::New(env);
      obj.Set("player", player);
#if SDL_VERSION_ATLEAST(2, 0, 14)
      auto code =
        SDL_GameControllerRumbleTriggers(controller.second, left_rumble,
                                         right_rumble, duration_ms);
#else
      (void) controller;
      (void) left_rumble;
      (void) right_rumble;
      (void) duration_ms;
      int code = -1;
#endif
      if (code >= 0) {
        emit({Napi::String::New(env, "rumbled-triggers"), obj});
      } else {
        obj.Set("message", SDL_GetError());
        obj.Set("operation", "rumbleTriggers");
        emit({Napi::String::New(env, "error"), obj});
      }
    }
  }
}

void SdlGameController::setLeds(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::Function emit_unbound =
    info.This().As<Napi::Object>().Get("emit").As<Napi::Function>();
  Napi::Function emit = emit_unbound.Get("bind")
                          .As<Napi::Function>()
                          .Call(emit_unbound, {info.This()})
                          .As<Napi::Function>();

  int playerNumber = 0;  // set LEDs for all players
  Uint8 red = 0x00;
  Uint8 green = 0x00;
  Uint8 blue = 0xff;
  auto warning = Napi::Object::New(env);

  // Check for red
  if (info.Length() > 0) {
    if (info[0].IsNumber()) {
      red = static_cast<uint32_t>(info[0].ToNumber());
    } else {
      warning.Set("message", "wrong argument type: red");
      emit({Napi::String::New(env, "warning"), warning});
    }
  }

  // Check for green
  if (info.Length() > 1) {
    if (info[1].IsNumber()) {
      green = static_cast<uint32_t>(info[1].ToNumber());
    } else {
      warning.Set("message", "wrong argument type: green");
      emit({Napi::String::New(env, "warning"), warning});
    }
  }

  // Check for blue
  if (info.Length() > 2) {
    if (info[2].IsNumber()) {
      blue = static_cast<uint32_t>(info[2].ToNumber());
    } else {
      warning.Set("message", "wrong argument type: blue");
      emit({Napi::String::New(env, "warning"), warning});
    }
  }

  // Check for player number
  if (info.Length() > 3) {
    if (info[3].IsNumber()) {
      playerNumber = info[3].ToNumber();
    } else {
      warning.Set("message", "wrong argument type: player");
      emit({Napi::String::New(env, "warning"), warning});
    }
  }

  for (auto &controller : gamecontrollers) {
#if SDL_VERSION_ATLEAST(2, 0, 12)
    auto player = SDL_GameControllerGetPlayerIndex(controller.second);
#else
    auto player = playerNumber;
#endif
    if (playerNumber == 0 || playerNumber == player) {
      auto obj = Napi::Object::New(env);
      obj.Set("player", player);
#if SDL_VERSION_ATLEAST(2, 0, 14)
      auto code = SDL_GameControllerSetLED(controller.second, red, green, blue);
#else
      (void) controller;
      (void) red;
      (void) green;
      (void) blue;
      int code = -1;
#endif
      if (code >= 0) {
        emit({Napi::String::New(env, "led"), obj});
      } else {
        obj.Set("message", SDL_GetError());
        obj.Set("operation", "setLeds");
        emit({Napi::String::New(env, "error"), obj});
      }
    }
  }
}
