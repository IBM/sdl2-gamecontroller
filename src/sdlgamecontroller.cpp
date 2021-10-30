/* copyright 2021 IBM */
#include "sdlgamecontroller.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_gamecontroller.h>
#include <chrono>
#include <set>

bool SdlGameController::sdlInit = false;

Napi::FunctionReference SdlGameController::constructor;

Napi::Object SdlGameController::Init(Napi::Env env, Napi::Object exports)
{
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(
      env, "SdlGameController",
      {InstanceMethod("pollEvents", &SdlGameController::pollEvents),
       InstanceMethod("enableGyroscope", &SdlGameController::enableGyroscope),
       InstanceMethod("enableAccelerometer",
                      &SdlGameController::enableAccelerometer),
       InstanceMethod("setLeds", &SdlGameController::setLeds),
       InstanceMethod("rumble", &SdlGameController::rumble),
       InstanceMethod("rumbleTriggers", &SdlGameController::rumbleTriggers)});

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("SdlGameController", func);

  return exports;
}

SdlGameController::SdlGameController(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<SdlGameController>(info)
{
  // NOOP
}

/* PS5 trigger effect documentation:
   https://controllers.fandom.com/wiki/Sony_DualSense#FFB_Trigger_Modes
*/
typedef struct
{
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

bool SdlGameController::SendEffect(SDL_GameController *gamecontroller)
{
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
  (void)gamecontroller;
  return SDL_FALSE;
#endif
}

void SdlGameController::RemoveController(const SDL_JoystickID which)
{
  auto search = gamecontrollers.find(which);
  if (search != gamecontrollers.end())
  {
    gamecontrollers.erase(search);
  }
}
int SdlGameController::NextPlayer()
{
#if SDL_VERSION_ATLEAST(2, 0, 12)
  // find current players
  std::set<int> players;
  for (auto &controller : gamecontrollers)
  {
    auto player = SDL_GameControllerGetPlayerIndex(controller.second);
    players.insert(player);
  }

  // Find the first avilable player slot. Max number of players is 8 probably.
  for (int player = 1; player < 8; player++)
  {
    auto search = players.find(player);
    if (search == players.end())
    {
      return player;
    }
  }
#endif
  return -1;
}

Napi::Value SdlGameController::pollEvents(const Napi::CallbackInfo &info)
{
  // do not spend too long here
  auto start = std::chrono::system_clock::now();

  Napi::Env env = info.Env();
  Napi::Function emit_unbound =
      info.This().As<Napi::Object>().Get("emit").As<Napi::Function>();
  Napi::Function emit = emit_unbound.Get("bind")
                            .As<Napi::Function>()
                            .Call(emit_unbound, {info.This()})
                            .As<Napi::Function>();

  // Set up SDL
  if (!SdlGameController::sdlInit)
  {
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
    if (SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) < 0)
    {
      emit({Napi::String::New(env, "error"),
            Napi::String::New(env, SDL_GetError())});
    }
    else
    {
      emit({Napi::String::New(env, "sdl-init")});
      SdlGameController::sdlInit = true;
    }
  }

  // poll until all events are handled!
  SDL_Event event;
  while (SDL_PollEvent(&event))
  {
    auto obj = Napi::Object::New(env);

    // only collect events for a "while". If we take too long just quit.
    auto now = std::chrono::system_clock::now();
    auto msSofar =
        std::chrono::duration_cast<std::chrono::milliseconds>(now - start)
            .count();
    if (msSofar > 100)
    {
      obj.Set("message", "Polling is taking too long.");
      obj.Set("elapsed_ms", msSofar);
      emit({Napi::String::New(env, "warning"), obj});
      break;
    }

    SDL_GameController *gc;
    std::string gcBtn;

    switch (event.type)
    {
    case SDL_CONTROLLERDEVICEADDED:
      obj.Set("message",
              "A new Game controller has been inserted into the system");

      gc = SDL_GameControllerOpen(event.cdevice.which);
      if (gc)
      {
        obj.Set("which", static_cast<int>(event.cdevice.which));
        obj.Set("name", SDL_GameControllerName(gc));
        obj.Set("vendor_id", SDL_GameControllerGetVendor(gc));
        obj.Set("product_id", SDL_GameControllerGetProduct(gc));

#if SDL_VERSION_ATLEAST(2, 0, 14)
        auto sn = SDL_GameControllerGetSerial(gc);
        if (sn)
          obj.Set("serial_number", sn);
        else
          obj.Set("serial_number", "none");
#endif

        auto js = SDL_GameControllerGetJoystick(gc);
        auto id = SDL_JoystickInstanceID(js);
        gamecontrollers[id] = gc;

#if SDL_VERSION_ATLEAST(2, 0, 12)
        auto player = NextPlayer();
        SDL_GameControllerSetPlayerIndex(gc, player);
        obj.Set("player", player);
#endif

        auto effects = SendEffect(gc) ? true : false;
        obj.Set("effects_supported", effects);

        auto isHaptic = SDL_JoystickIsHaptic(js) ? true : false;
        obj.Set("haptic", isHaptic);

#if SDL_VERSION_ATLEAST(2, 0, 14)
        auto hasLeds = SDL_GameControllerHasLED(gc) ? "true" : "false";
        obj.Set("has_leds", hasLeds);

        auto nTouchPads = SDL_GameControllerGetNumTouchpads(gc);
        obj.Set("num_touchpads", nTouchPads);

        auto accelerometer =
            SDL_GameControllerHasSensor(gc, SDL_SENSOR_ACCEL) ? true : false;
        obj.Set("has_accelerometer", accelerometer);

        auto gyroscope =
            SDL_GameControllerHasSensor(gc, SDL_SENSOR_GYRO) ? true : false;
        obj.Set("has_gyroscope", gyroscope);
#endif

        SDL_ClearError();
        // Range seems to be 0x0200 - 0xFFFC
#if SDL_VERSION_ATLEAST(2, 0, 10)
        auto rumble = SDL_GameControllerRumble(gc, 0x0200, 0x0200, 250) >= 0;
        obj.Set("has_rumble", rumble);
#endif
#if SDL_VERSION_ATLEAST(2, 0, 14)
        SDL_ClearError();
        auto trigger =
            SDL_GameControllerRumbleTriggers(gc, 0, 0xFFFC, 1000) > 0;
        obj.Set("has_rumble_trigger", trigger);
#endif
      }
      else
      {
        obj.Set("message", SDL_GetError());
        obj.Set("operation", "SDL_GameControllerOpen");
      }
      emit({Napi::String::New(env, "controller-device-added"), obj});
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
      if (gc)
      {
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
      if (gc)
      {
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
      if (gc)
      {
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
      switch (event.csensor.sensor)
      {
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
    }
  }

  return Napi::String::New(env, "OK");
}

void SdlGameController::enableGyroscope(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  Napi::Function emit_unbound =
      info.This().As<Napi::Object>().Get("emit").As<Napi::Function>();
  Napi::Function emit = emit_unbound.Get("bind")
                            .As<Napi::Function>()
                            .Call(emit_unbound, {info.This()})
                            .As<Napi::Function>();

  SDL_bool enable = SDL_TRUE;
  int playerNumber = 0; // enable for all players

  // Check if enable is set.
  if (info.Length() > 0)
  {
    // Check the argument types
    if (!info[0].IsBoolean())
    {
      throw Napi::Error::New(env, "Wrong arguments");
      return;
    }
    enable = info[0].ToBoolean() ? SDL_TRUE : SDL_FALSE;
  };

  // Check if player number is set
  if (info.Length() > 1)
  {
    // Check the argument types
    if (!info[0].IsNumber())
    {
      throw Napi::Error::New(env, "Wrong arguments");
      return;
    }
    playerNumber = info[1].ToNumber();
  };

  for (auto &controller : gamecontrollers)
  {
    auto obj = Napi::Object::New(env);
#if SDL_VERSION_ATLEAST(2, 0, 12)
    auto player = SDL_GameControllerGetPlayerIndex(controller.second);
    obj.Set("player", player);
#else
    auto player = playerNumber;
#endif
    if (playerNumber == 0 || playerNumber == player)
    {
#if SDL_VERSION_ATLEAST(2, 0, 14)
      auto success = SDL_GameControllerSetSensorEnabled(
                         controller.second, SDL_SENSOR_GYRO, enable) == 0;
#else
      (void)controller;
      auto success = false;
#endif
      if (success)
      {
        if (enable)
          emit({Napi::String::New(env, "gyroscope:enabled"), obj});
        else
          emit({Napi::String::New(env, "gyroscope:disabled"), obj});
      }
      else
      {
        obj.Set("message", SDL_GetError());
        obj.Set("operation", "enableGyroscope");
        emit({Napi::String::New(env, "error"), obj});
      }
    }
  }
}

void SdlGameController::enableAccelerometer(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  Napi::Function emit_unbound =
      info.This().As<Napi::Object>().Get("emit").As<Napi::Function>();
  Napi::Function emit = emit_unbound.Get("bind")
                            .As<Napi::Function>()
                            .Call(emit_unbound, {info.This()})
                            .As<Napi::Function>();

  int playerNumber = 0; // enable for all players
  SDL_bool enable = SDL_TRUE;

  // Check the number of arguments passed.
  if (info.Length() > 0)
  {
    // Check the argument types
    if (!info[0].IsBoolean())
    {
      throw Napi::Error::New(env, "Wrong arguments");
      return;
    }
    enable = info[0].ToBoolean() ? SDL_TRUE : SDL_FALSE;
  };

  if (info.Length() > 1)
  {
    // Check the argument types
    if (!info[1].IsNumber())
    {
      throw Napi::Error::New(env, "Wrong arguments");
      return;
    }
    playerNumber = info[1].ToNumber();
  };

  for (auto &controller : gamecontrollers)
  {
#if SDL_VERSION_ATLEAST(2, 0, 12)
    auto player = SDL_GameControllerGetPlayerIndex(controller.second);
#else
    auto player = playerNumber;
#endif
    if (playerNumber == 0 || playerNumber == player)
    {
      auto obj = Napi::Object::New(env);
      obj.Set("player", player);
#if SDL_VERSION_ATLEAST(2, 0, 14)
      auto success = SDL_GameControllerSetSensorEnabled(
                         controller.second, SDL_SENSOR_ACCEL, enable) == 0;
#else
      (void)controller;
      auto success = false;
#endif
      if (success)
      {
        if (enable)
          emit({Napi::String::New(env, "accelerometer:enabled"), obj});
        else
          emit({Napi::String::New(env, "accelerometer:disabled"), obj});
      }
      else
      {
        obj.Set("message", SDL_GetError());
        obj.Set("operation", "enableAccelerometer");
        emit({Napi::String::New(env, "error"), obj});
      }
    }
  }
}

void SdlGameController::rumble(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  Napi::Function emit_unbound =
      info.This().As<Napi::Object>().Get("emit").As<Napi::Function>();
  Napi::Function emit = emit_unbound.Get("bind")
                            .As<Napi::Function>()
                            .Call(emit_unbound, {info.This()})
                            .As<Napi::Function>();

  int playerNumber = 0; // rumble all players
  Uint16 low_frequency_rumble = 0xFFFC;
  Uint16 high_frequency_rumble = 0xFFFC;
  Uint32 duration_ms = 250;

  // Check for low frequency
  if (info.Length() > 0)
  {
    // Check the argument type
    if (!info[0].IsNumber())
    {
      throw Napi::Error::New(env, "Wrong arguments");
      return;
    }
    low_frequency_rumble = static_cast<uint32_t>(info[0].ToNumber());
  };

  // Check for high frequency
  if (info.Length() > 1)
  {
    // Check the argument type
    if (!info[1].IsNumber())
    {
      throw Napi::Error::New(env, "Wrong arguments");
      return;
    }
    high_frequency_rumble = static_cast<uint32_t>(info[1].ToNumber());
  };

  // Check for high frequency
  if (info.Length() > 2)
  {
    // Check the argument type
    if (!info[2].IsNumber())
    {
      throw Napi::Error::New(env, "Wrong arguments");
      return;
    }
    duration_ms = static_cast<uint32_t>(info[2].ToNumber());
  };

  // Check for player number
  if (info.Length() > 3)
  {
    // Check the argument types
    if (!info[3].IsNumber())
    {
      throw Napi::Error::New(env, "Wrong arguments");
      return;
    }
    playerNumber = info[3].ToNumber();
  };

  for (auto &controller : gamecontrollers)
  {
#if SDL_VERSION_ATLEAST(2, 0, 12)
    auto player = SDL_GameControllerGetPlayerIndex(controller.second);
#else
    auto player = playerNumber;
#endif
    if (playerNumber == 0 || playerNumber == player)
    {
      auto obj = Napi::Object::New(env);
      obj.Set("player", player);
#if SDL_VERSION_ATLEAST(2, 0, 10)
      auto success =
          SDL_GameControllerRumble(controller.second, low_frequency_rumble,
                                   high_frequency_rumble, duration_ms);
#else
      (void)controller;
      (void)low_frequency_rumble;
      (void)high_frequency_rumble;
      (void)duration_ms;
      int success = -1;
#endif
      if (success >= 0)
      {
        emit({Napi::String::New(env, "rumbled"), obj});
      }
      else
      {
        obj.Set("message", SDL_GetError());
        obj.Set("operation", "rumble");
        emit({Napi::String::New(env, "error"), obj});
      }
    }
  }
}

void SdlGameController::rumbleTriggers(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  Napi::Function emit_unbound =
      info.This().As<Napi::Object>().Get("emit").As<Napi::Function>();
  Napi::Function emit = emit_unbound.Get("bind")
                            .As<Napi::Function>()
                            .Call(emit_unbound, {info.This()})
                            .As<Napi::Function>();

  int playerNumber = 0; // rumble all players
  Uint16 left_rumble = 0xFFFC;
  Uint16 right_rumble = 0xFFFC;
  Uint32 duration_ms = 250;

  // Check for left
  if (info.Length() > 0)
  {
    // Check the argument type
    if (!info[0].IsNumber())
    {
      throw Napi::Error::New(env, "Wrong arguments");
      return;
    }
    left_rumble = static_cast<uint32_t>(info[0].ToNumber());
  };

  // Check for right
  if (info.Length() > 1)
  {
    // Check the argument type
    if (!info[1].IsNumber())
    {
      throw Napi::Error::New(env, "Wrong arguments");
      return;
    }
    right_rumble = static_cast<uint32_t>(info[1].ToNumber());
  };

  // Check for duration
  if (info.Length() > 2)
  {
    // Check the argument type
    if (!info[2].IsNumber())
    {
      throw Napi::Error::New(env, "Wrong arguments");
      return;
    }
    duration_ms = static_cast<uint32_t>(info[2].ToNumber());
  };

  // Check for player number
  if (info.Length() > 3)
  {
    // Check the argument types
    if (!info[3].IsNumber())
    {
      throw Napi::Error::New(env, "Wrong arguments");
      return;
    }
    playerNumber = info[3].ToNumber();
  };

  for (auto &controller : gamecontrollers)
  {
#if SDL_VERSION_ATLEAST(2, 0, 12)
    auto player = SDL_GameControllerGetPlayerIndex(controller.second);
#else
    auto player = playerNumber;
#endif
    if (playerNumber == 0 || playerNumber == player)
    {
      auto obj = Napi::Object::New(env);
      obj.Set("player", player);
#if SDL_VERSION_ATLEAST(2, 0, 14)
      auto code = SDL_GameControllerRumbleTriggers(
          controller.second, left_rumble, right_rumble, duration_ms);
#else
      (void)controller;
      (void)left_rumble;
      (void)right_rumble;
      (void)duration_ms;
      int code = -1;
#endif
      if (code >= 0)
      {
        emit({Napi::String::New(env, "rumbled-triggers"), obj});
      }
      else
      {
        obj.Set("message", SDL_GetError());
        obj.Set("operation", "rumbleTriggers");
        emit({Napi::String::New(env, "error"), obj});
      }
    }
  }
}

void SdlGameController::setLeds(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  Napi::Function emit_unbound =
      info.This().As<Napi::Object>().Get("emit").As<Napi::Function>();
  Napi::Function emit = emit_unbound.Get("bind")
                            .As<Napi::Function>()
                            .Call(emit_unbound, {info.This()})
                            .As<Napi::Function>();

  int playerNumber = 0; // set LEDs for all players
  Uint8 red = 0x00;
  Uint8 green = 0x00;
  Uint8 blue = 0xff;

  // Check for red
  if (info.Length() > 0)
  {
    // Check the argument type
    if (!info[0].IsNumber())
    {
      throw Napi::Error::New(env, "Wrong arguments");
      return;
    }
    red = static_cast<uint32_t>(info[0].ToNumber());
  };

  // Check for green
  if (info.Length() > 1)
  {
    // Check the argument type
    if (!info[1].IsNumber())
    {
      throw Napi::Error::New(env, "Wrong arguments");
      return;
    }
    green = static_cast<uint32_t>(info[1].ToNumber());
  };

  // Check for blue
  if (info.Length() > 2)
  {
    // Check the argument type
    if (!info[2].IsNumber())
    {
      throw Napi::Error::New(env, "Wrong arguments");
      return;
    }
    blue = static_cast<uint32_t>(info[2].ToNumber());
  };

  // Check for player number
  if (info.Length() > 3)
  {
    // Check the argument types
    if (!info[3].IsNumber())
    {
      throw Napi::Error::New(env, "Wrong arguments");
      return;
    }
    playerNumber = info[3].ToNumber();
  };

  for (auto &controller : gamecontrollers)
  {
#if SDL_VERSION_ATLEAST(2, 0, 12)
    auto player = SDL_GameControllerGetPlayerIndex(controller.second);
#else
    auto player = playerNumber;
#endif
    if (playerNumber == 0 || playerNumber == player)
    {
      auto obj = Napi::Object::New(env);
      obj.Set("player", player);
#if SDL_VERSION_ATLEAST(2, 0, 14)
      auto code = SDL_GameControllerSetLED(controller.second, red, green, blue);
#else
      (void)controller;
      (void)red;
      (void)green;
      (void)blue;
      int code = -1;
#endif
      if (code >= 0)
      {
        emit({Napi::String::New(env, "led"), obj});
      }
      else
      {
        obj.Set("message", SDL_GetError());
        obj.Set("operation", "setLeds");
        emit({Napi::String::New(env, "error"), obj});
      }
    }
  }
}
