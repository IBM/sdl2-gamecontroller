const gamecontroller = require("../index.js");

gamecontroller.on("error", (data) => console.log("error", data));
gamecontroller.on("warning", (data) => console.log("warning", data));
gamecontroller.on("sdl-init", () => console.log("SDL2 Initialized"));
gamecontroller.on("a:down", (data) => console.log("Hello A button world"));

gamecontroller.on("x:down", (data) => process.exit(0));
