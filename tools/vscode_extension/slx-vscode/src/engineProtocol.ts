// Messages exchanged between the extension and the engine worker thread.

export type FunctionBehaviour = 'nodedef' | 'nodegraph' | 'inline' | 'comptime';

export interface CompileSettings {
    materialxVersion: string;
    reduceGraph: boolean;
    defaultFunctionBehaviour: FunctionBehaviour;
    errorOnMissingGlobals: boolean;
}

export interface InMemorySource {
    key: string;
    text: string;
}

export interface CompileRequest {
    id: number;
    kind: 'compile';
    source: string;
    sources: InMemorySource[];
    settings: CompileSettings;
}

export interface DecompileRequest {
    id: number;
    kind: 'decompile';
    source: string;
}

export type EngineRequest = CompileRequest | DecompileRequest;

export interface EngineSuccess {
    kind: 'result';
    id: number;
    ok: true;
    output: string;
}

export interface EngineFailure {
    kind: 'result';
    id: number;
    ok: false;
    errorName: string;
    errorMessage: string;
}

/** Text the compiler printed while handling a request, from `print` statements (stdout) or warnings (stderr). */
export interface EngineLog {
    kind: 'log';
    id: number;
    stream: 'stdout' | 'stderr';
    text: string;
}

export type EngineMessage = EngineSuccess | EngineFailure | EngineLog;

export interface EngineWorkerData {
    /** Folder holding JsMxslc.mjs, JsMxslc.wasm and JsMxslc.data. */
    libDir: string;
}

/** Error name used when the WebAssembly module could not be loaded. */
export const ENGINE_LOAD_ERROR = 'EngineLoadError';
