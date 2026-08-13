import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';

// Absolute filesystem directory that holds the staged WASM build artifacts
// (JsMxslc.js / .wasm / .data), copied there by scripts/buildSetup.js.
const BUILD_DIR = fileURLToPath(new URL('./_build/', import.meta.url));

// Read a set of text files from a directory relative to the test folder.
export function getFileStrings(fileNames, subPath)
{
    return fileNames.map(name => fs.readFileSync(path.resolve(subPath, name), 'utf8'));
}

// Load the Emscripten module. The module factory returns a promise that
// resolves to the mxslc API object once the WASM binary and the preloaded
// data file (which contains the libraries/ folder) have been loaded.
//
// In Node.js the runtime locates the .wasm/.data files relative to the process
// working directory, so we override locateFile to point at the staged _build
// folder using a plain filesystem path.
export async function loadMxslc()
{
    const { default: Module } = await import('./_build/JsMxslc.js');
    return await Module({
        locateFile: (file) => BUILD_DIR + file,
    });
}
