declare const gamecontroller: Gamecontroller;

export default gamecontroller;

export type Message = { message: string };
export type Player = { player: number };
export type Error = Message & Player & { operation: string };
export type Value = { value: number };
export type TimeStamp = { timestamp: number };
export type OnButtonPress = Message &
  Player & { button: ButtonType; pressed: boolean };

export type OnTriggerPress = Omit<OnButtonPress, 'pressed'> & {
  value: number;
  timestamp: number;
};
export type StickType = 'leftx' | 'lefty' | 'rightx' | 'righty';

export type OnStickMoveData = Message &
  Player &
  Value &
  TimeStamp & { button: StickType };

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

export type ButtonTypeWithUpsAndDowns =
  | `${ButtonType}:up`
  | `${ButtonType}:down`
  | ButtonType;

export type ControllerButtonDown = Message &
  Player & { button: ButtonType; pressed: boolean };

export type FieldUpdateHandler = (update: any) => unknown;
export type Handlers = Record<string, FieldUpdateHandler>;
export type CallBack<T = Record<string, unknown>> = (data: T) => void;

type ON<TEventName, TCallBack> = (
  eventName: TEventName,
  callBack: CallBack<TCallBack>
) => void;

type OnStickMove = ON<StickType, OnStickMoveData>;
type OnButtonPressCall = ON<ButtonTypeWithUpsAndDowns, OnButtonPress>;
type OnErrorCall = ON<'error', Error>;
type OnControllerButtonUp = ON<'controller-button-up', OnButtonPress>;
type OnControllerButtonDown = ON<'controller-button-down', OnButtonPress>;
type OnTrigger = ON<'righttrigger' | 'lefttrigger', OnTriggerPress>;

type AllOnOptions = OnButtonPressCall &
  OnStickMove &
  OnErrorCall &
  OnControllerButtonUp &
  OnControllerButtonDown &
  OnTrigger;

type Gamecontroller = {
  rumbleTriggers: (
    buttonType: ButtonType,
    callBack: CallBack<OnTriggerPress>
  ) => void;
  on: AllOnOptions;
};
