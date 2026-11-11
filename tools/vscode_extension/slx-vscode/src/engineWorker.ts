// Worker thread that runs the mxslc WebAssembly module.
//
// Compiling is synchronous and large shaders take a few hundred milliseconds,
// so it runs here rather than on the extension host thread, which every
// extension shares. Requests are handled one at a time, in order.
import * as path from 'node:path';
import { pathToFileURL } from 'node:url';
import { parentPort, workerData } from 'node:worker_threads';
import {
    ENGINE_LOAD_ERROR,
    type CompileRequest,
    type EngineMessage,
    type EngineRequest,
    type EngineWorkerData,
    type FunctionBehaviour,
} from './engineProtocol';

// The parts of the Emscripten module this worker uses. See mxslc++/javascript/JsMxslc.cpp.
interface CompileOptions {
    version: string;
    reduceGraph: boolean;
    errorOnMissingGlobals: boolean;
    defaultFunctionBehaviour: unknown;
    addSource(name: string, contents: string): void;
    delete(): void;
}

interface MxslcModule {
    CompileOptions: new () => CompileOptions;
    FunctionBehaviour: Record<'NodeDef' | 'NodeGraph' | 'Inline' | 'Comptime', unknown>;
    compileSlxToMtlx(source: string, opts: CompileOptions): string;
    decompileMtlxToSlx(source: string): string;
}

const port = parentPort!;
const { libDir } = workerData as EngineWorkerData;

// The request being handled, so printed text can be sent with its id.
let currentId = 0;

function post(message: EngineMessage): void {
    port.postMessage(message);
}

async function loadModule(): Promise<MxslcModule> {
    const moduleUrl = pathToFileURL(path.join(libDir, 'JsMxslc.mjs')).href;
    const factory = (await import(moduleUrl)).default;
    return factory({
        // Absolute paths, so the .wasm and .data files are found whatever the
        // working directory is.
        locateFile: (file: string) => path.join(libDir, file),
        print: (text: string) => post({ kind: 'log', id: currentId, stream: 'stdout', text }),
        printErr: (text: string) => post({ kind: 'log', id: currentId, stream: 'stderr', text }),
    });
}

const modulePromise = loadModule();

function toFunctionBehaviour(mx: MxslcModule, behaviour: FunctionBehaviour): unknown {
    switch (behaviour) {
        case 'nodegraph':
            return mx.FunctionBehaviour.NodeGraph;
        case 'inline':
            return mx.FunctionBehaviour.Inline;
        case 'comptime':
            return mx.FunctionBehaviour.Comptime;
        default:
            return mx.FunctionBehaviour.NodeDef;
    }
}

function compile(mx: MxslcModule, request: CompileRequest): string {
    const opts = new mx.CompileOptions();
    try {
        opts.version = request.settings.materialxVersion;
        opts.reduceGraph = request.settings.reduceGraph;
        opts.errorOnMissingGlobals = request.settings.errorOnMissingGlobals;
        opts.defaultFunctionBehaviour = toFunctionBehaviour(mx, request.settings.defaultFunctionBehaviour);
        for (const source of request.sources) opts.addSource(source.key, source.text);
        return mx.compileSlxToMtlx(request.source, opts);
    } finally {
        opts.delete();
    }
}

function describeError(error: unknown): { errorName: string; errorMessage: string } {
    if (error instanceof Error) return { errorName: error.name, errorMessage: error.message };
    return { errorName: 'Error', errorMessage: String(error) };
}

port.on('message', async (request: EngineRequest) => {
    let mx: MxslcModule;
    try {
        mx = await modulePromise;
    } catch (error) {
        post({
            kind: 'result',
            id: request.id,
            ok: false,
            errorName: ENGINE_LOAD_ERROR,
            errorMessage: `Could not load the SLX compiler from ${libDir}: ${describeError(error).errorMessage}`,
        });
        return;
    }

    currentId = request.id;
    try {
        const output = request.kind === 'compile' ? compile(mx, request) : mx.decompileMtlxToSlx(request.source);
        post({ kind: 'result', id: request.id, ok: true, output });
    } catch (error) {
        post({ kind: 'result', id: request.id, ok: false, ...describeError(error) });
    }
});
