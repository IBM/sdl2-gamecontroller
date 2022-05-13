import {Gamecontroller} from './index';

export type GameControllerOptions = {
  interval?: Number;
  fps?: Number;
  sdl_joystick_rog_chakram?: Boolean;
};
export type CustomController = (
  options: GameControllerOptions,
) => Gamecontroller;

declare const customController: CustomController;
export default customController;
