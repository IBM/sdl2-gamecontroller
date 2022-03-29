export type Message = {message: string};
export type Player = {player?: number};
export type Error = Message & Player & {operation: string};
export type Warning = Message & {
  operation: string;
  elapsed_ms?: number;
  poll_number?: number;
};
export type SdlInit = {
  compiled_against_SDL_version: string;
  linkeded_against_SDL_version: string;
};
export type DeviceAdded = Message &
  Player & {
    which: number;
    name: string;
    vendor_id: number;
    product_id: number;
    serial_number?: string;
    effects_supported: boolean;
    haptic: boolean;
    has_leds?: boolean;
    num_touchpads?: number;
    has_accelerometer?: boolean;
    has_gyroscope?: boolean;
    has_rumble?: boolean;
    has_rumble_trigger?: boolean;
  };
export type SensorUpdate = Message & {
  sensor: 'gyroscope' | 'accelerometer';
  x: number;
  y: number;
  z: number;
};
export type SensorUpdateEvents =
  | 'controller-sensor-update'
  | 'gyroscope'
  | 'accelerometer';
export type SensorStateEvents =
  | 'accelerometer:enabled'
  | 'accelerometer:disabled'
  | 'gyroscope:enabled'
  | 'gyroscope:disabled';

export type TouchpadUpdate = Message & {
  touchpad: number;
  finger: number;
  x: number;
  y: number;
  pressure: number;
};
export type TouchpadEvents =
  | 'controller-touchpad-down'
  | 'controller-touchpad-up'
  | 'controller-touchpad-motion';

export type DeviceUpdated = Message & {
  which: number;
};
export type DeviceUpdateEvents =
  | 'controller-device-removed'
  | 'controller-device-remapped';

export type AxisType =
  | 'leftx'
  | 'lefty'
  | 'rightx'
  | 'righty'
  | 'lefttrigger'
  | 'righttrigger';

export type ButtonType =
  | 'a'
  | 'b'
  | 'x'
  | 'y'
  | 'back'
  | 'guide'
  | 'start'
  | 'leftstick'
  | 'misc1'
  | 'rightstick'
  | 'leftshoulder'
  | 'rightshoulder'
  | 'dpup'
  | 'dpdown'
  | 'dpright'
  | 'dpleft';

export type AxisMotionData = Message &
  Player & {button: AxisType; value: number; timestamp: number};

export type ButtonPress = Message &
  Player & {button: ButtonType; pressed: boolean};

export type ButtonTypeWithUpsAndDowns =
  | `${ButtonType}:up`
  | `${ButtonType}:down`
  | ButtonType
  | 'controller-button-up'
  | 'controller-button-down';

export type ControllerButtonDown = Message &
  Player & {button: ButtonType; pressed: boolean};

export type CallBack<T = Record<string, unknown>> = (data: T) => void;

type ON<TEventName, TCallBack> = (
  eventName: TEventName,
  callBack: CallBack<TCallBack>,
) => void;

type OnErrorCall = ON<'error', Error>;
type OnWarningCall = ON<'warning', Warning>;
type OnSdlInitCall = ON<'sdl-init', SdlInit>;
type OnDeviceAddedCall = ON<'controller-device-added', DeviceAdded>;
type OnDeviceUpdated = ON<DeviceUpdateEvents, DeviceUpdated>;
type OnAxisUpdate = ON<AxisType, AxisMotionData>;
type OnButtonPressCall = ON<ButtonTypeWithUpsAndDowns, ButtonPress>;
type OnSensorUpdate = ON<SensorUpdateEvents, SensorUpdate>;
type OnSensorStateChange = ON<SensorStateEvents, Player>;
type OnTouchpadUpdate = ON<TouchpadEvents, TouchpadUpdate>;
type OnLed = ON<'led', Player>;
type OnRumbled = ON<'rumbled', Player>;
type OnRumbledTriggers = ON<'rumbled-triggers', Player>;

type AllOnOptions = OnButtonPressCall &
  OnAxisUpdate &
  OnErrorCall &
  OnWarningCall &
  OnSdlInitCall &
  OnDeviceAddedCall &
  OnDeviceUpdated &
  OnSensorUpdate &
  OnSensorStateChange &
  OnTouchpadUpdate &
  OnLed &
  OnRumbled &
  OnRumbledTriggers;

type Gamecontroller = {
  enableGyroscope: (enable?: boolean, player?: number) => void;
  enableAccelerometer: (enable?: boolean, player?: number) => void;
  setLeds: (
    red?: number,
    green?: number,
    blue?: number,
    player?: number,
  ) => void;
  rumble: (
    low_frequency_rumble?: number,
    high_frequency_rumble?: number,
    duration_ms?: number,
    player?: number,
  ) => void;
  rumbleTriggers: (
    left_rumble?: number,
    right_rumble?: number,
    duration_ms?: number,
    player?: number,
  ) => void;
  on: AllOnOptions;
};

declare const gamecontroller: Gamecontroller;
export default gamecontroller;
