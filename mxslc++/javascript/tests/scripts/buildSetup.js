// Cross-platform clean and copy of build artifacts for testing, used as
// Playwright's globalSetup. It recursively searches the javascript/build
// folder for the emscripten output (JsMxslc.js / .wasm / .data) and stages a
// copy into ./_build so tests can import it from a fixed location.
import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const testRoot = path.resolve(__dirname, '..');
const buildDir = path.join(testRoot, '_build');
const buildRoot = path.resolve(testRoot, '..', 'build');

function findWasmFiles(dir, results = [])
{
    if (!fs.existsSync(dir))
    {
        return results;
    }
    for (const entry of fs.readdirSync(dir, { withFileTypes: true }))
    {
        const full = path.join(dir, entry.name);
        if (entry.isDirectory())
        {
            findWasmFiles(full, results);
        }
        else if (/^JsMxslc\.(js|wasm|data)$/.test(entry.name))
        {
            results.push(full);
        }
    }
    return results;
}

export default function buildSetup()
{
    fs.rmSync(buildDir, { recursive: true, force: true });
    fs.mkdirSync(buildDir, { recursive: true });

    const files = findWasmFiles(buildRoot);
    if (files.length === 0)
    {
        throw new Error(
            `No JsMxslc.* build output found under ${buildRoot}. ` +
            'Run javascript/build_javascript.sh first.'
        );
    }
    for (const file of files)
    {
        fs.copyFileSync(file, path.join(buildDir, path.basename(file)));
    }
}
