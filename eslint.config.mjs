import js from '@eslint/js';
import globals from 'globals';
import tseslint from 'typescript-eslint';
import stylistic from '@stylistic/eslint-plugin';
import { defineConfig } from 'eslint/config';

export default defineConfig([
  {
    ignores: ['build/**', 'cmake*/**', 'test/build/**', 'dist/**'],
  },
  {
    files: ['**/*.{js,mjs,cjs,ts,mts,cts}'],
    plugins: { js, '@stylistic': stylistic },
    extends: ['js/recommended'],
    languageOptions: { globals: globals.nodeBuiltin },
  },
  tseslint.configs.recommended,
  stylistic.configs.customize({
    semi: true,
    arrowParens: 'always',
  }),
  {
    rules: {
      '@typescript-eslint/ban-ts-comment': 'off',
    },
  },
]);
