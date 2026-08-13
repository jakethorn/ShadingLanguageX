import { defineConfig } from '@playwright/test';

export default defineConfig({
    // Stage the WASM build artifacts into _build before any project runs.
    globalSetup: './scripts/buildSetup.js',
    // Write test artifacts under _build, which is gitignored.
    outputDir: './_build/test-results',
    timeout: 120_000,
    projects: [
        {
            // Node-side unit tests for the JsMxslc bindings. These run in the
            // Playwright worker process (Node) and never launch a browser.
            name: 'unit',
            testDir: '.',
            testIgnore: ['**/node_modules/**'],
        },
    ],
});
