#!/usr/bin/env bash
set -x
npm rm sdl2-gamecontroller
rm -f ../sdl2-gamecontroller-*.tgz
pushd ..
npm run build
npm pack
popd
npm i ../sdl2-gamecontroller-*.tgz
rm -rf build
npx tsc --outDir build helloworld.ts helloworld-custom.ts lengthy.ts
