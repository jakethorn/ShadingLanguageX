// Builds the extension with esbuild.
//
//   node esbuild.mjs               development build into dist/
//   node esbuild.mjs --production  minified build into dist/ (used by vsce)
//   node esbuild.mjs --watch       rebuild dist/ on every change
//   node esbuild.mjs --tests       build the unit tests into out/test/
//
// The extension itself is bundled to CommonJS, which is what VS Code loads.
// The engine worker is bundled to an ES module because it imports the
// Emscripten-generated JsMxslc.mjs, and only an ES module can do that.
import * as esbuild from 'esbuild';
import { readdirSync } from 'node:fs';

const production = process.argv.includes('--production');
const watch = process.argv.includes('--watch');
const tests = process.argv.includes('--tests');

const common = {
    bundle: true,
    platform: 'node',
    target: 'node18',
    sourcemap: production ? false : 'linked',
    minify: production,
    logLevel: 'info',
};

const builds = tests
    ? [
          {
              ...common,
              entryPoints: readdirSync('test')
                  .filter((name) => name.endsWith('.test.ts'))
                  .map((name) => `test/${name}`),
              outdir: 'out/test',
              format: 'cjs',
          },
      ]
    : [
          {
              ...common,
              entryPoints: ['src/extension.ts'],
              outfile: 'dist/extension.js',
              format: 'cjs',
              external: ['vscode'],
          },
          {
              ...common,
              entryPoints: ['src/engineWorker.ts'],
              outfile: 'dist/engineWorker.mjs',
              format: 'esm',
          },
      ];

if (watch) {
    for (const options of builds) {
        const context = await esbuild.context(options);
        await context.watch();
    }
} else {
    await Promise.all(builds.map((options) => esbuild.build(options)));
}
