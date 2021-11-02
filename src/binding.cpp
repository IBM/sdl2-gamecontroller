#include "sdlgamecontroller.h"
#include <napi.h>  // NOLINT

Napi::Object Init(Napi::Env env, Napi::Object exports) 
{
  SdlGameController::Init(env, exports);
  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)
