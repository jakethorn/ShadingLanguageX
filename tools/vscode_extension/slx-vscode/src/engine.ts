// Runs the mxslc compiler and decompiler in a worker thread.
//
// The worker is started on the first request. If it crashes, pending requests
// fail and the next request starts a new worker.
//
// The worker is also replaced after a number of failed compiles. Failed
// compiles leave state behind in the WebAssembly module: after around 1500 of
// them, each successful compile leaks a few hundred kilobytes and gets slower
// until the module is unusable. Checking a file as it is typed fails often, so
// a long editing session would get there. Starting a new worker takes about
// 150 ms.
import * as fs from 'node:fs';
import * as path from 'node:path';
import { Worker } from 'node:worker_threads';
import {
    ENGINE_LOAD_ERROR,
    type CompileSettings,
    type EngineMessage,
    type EngineRequest,
    type EngineWorkerData,
    type InMemorySource,
} from './engineProtocol';

export const ENGINE_MISSING_ERROR = 'EngineMissingError';
export const ENGINE_CRASH_ERROR = 'EngineCrashError';

export class EngineError extends Error {
    constructor(name: string, message: string) {
        super(message);
        this.name = name;
    }

    /** True when the compiler itself is unavailable, rather than the source having an error. */
    get isEngineFailure(): boolean {
        return this.name === ENGINE_LOAD_ERROR || this.name === ENGINE_CRASH_ERROR || this.name === ENGINE_MISSING_ERROR;
    }
}

export type EngineLogger = (stream: 'stdout' | 'stderr', text: string) => void;

/** Failed requests after which the worker is replaced, well below where problems start. */
const FAILURES_BEFORE_RESTART = 200;

interface PendingRequest {
    resolve(output: string): void;
    reject(error: EngineError): void;
    log?: EngineLogger;
}

type RequestBody = { kind: 'compile'; source: string; sources: InMemorySource[]; settings: CompileSettings } | { kind: 'decompile'; source: string };

export class SlxEngine {
    private worker: Worker | undefined;
    private readonly pending = new Map<number, PendingRequest>();
    private nextId = 1;
    private failuresInWorker = 0;

    /**
     * @param workerPath path of the bundled engineWorker.mjs
     * @param libDir folder holding JsMxslc.mjs, JsMxslc.wasm and JsMxslc.data
     * @param failuresBeforeRestart failed requests after which the worker is replaced
     */
    constructor(
        private readonly workerPath: string,
        private readonly libDir: string,
        private readonly failuresBeforeRestart = FAILURES_BEFORE_RESTART,
    ) {}

    /** Number of workers started so far. */
    workersStarted = 0;

    /** True when the WebAssembly files are present. */
    get isBundled(): boolean {
        return fs.existsSync(path.join(this.libDir, 'JsMxslc.mjs'));
    }

    /** Compiles SLX to MaterialX. Text the compiler prints while doing so goes to log. */
    compile(source: string, sources: InMemorySource[], settings: CompileSettings, log?: EngineLogger): Promise<string> {
        return this.send({ kind: 'compile', source, sources, settings }, log);
    }

    /** Decompiles MaterialX to SLX. Text the decompiler prints while doing so goes to log. */
    decompile(source: string, log?: EngineLogger): Promise<string> {
        return this.send({ kind: 'decompile', source }, log);
    }

    dispose(): void {
        const worker = this.worker;
        this.worker = undefined;
        this.failPending('The SLX compiler was stopped.');
        void worker?.terminate();
    }

    private send(body: RequestBody, log: EngineLogger | undefined): Promise<string> {
        if (!this.isBundled) {
            return Promise.reject(
                new EngineError(
                    ENGINE_MISSING_ERROR,
                    `The SLX compiler is not bundled with this build of the extension (expected it in ${this.libDir}).`,
                ),
            );
        }
        const worker = this.startWorker();
        const id = this.nextId++;
        return new Promise<string>((resolve, reject) => {
            this.pending.set(id, { resolve, reject, log });
            // Keep the process alive while a request is in flight.
            worker.ref();
            worker.postMessage({ ...body, id } as EngineRequest);
        });
    }

    private startWorker(): Worker {
        if (this.worker) return this.worker;

        const workerData: EngineWorkerData = { libDir: this.libDir };
        const worker = new Worker(this.workerPath, { workerData });
        this.workersStarted++;
        this.failuresInWorker = 0;
        worker.on('message', (message: EngineMessage) => this.handleMessage(message));
        worker.on('error', (error) => {
            if (this.worker !== worker) return;
            this.worker = undefined;
            this.failPending(`The SLX compiler crashed: ${error.message}`);
        });
        worker.on('exit', (code) => {
            if (this.worker !== worker) return;
            this.worker = undefined;
            this.failPending(`The SLX compiler stopped unexpectedly (exit code ${code}).`);
        });
        this.worker = worker;
        return worker;
    }

    private handleMessage(message: EngineMessage): void {
        if (message.kind === 'log') {
            this.pending.get(message.id)?.log?.(message.stream, message.text);
            return;
        }
        const request = this.pending.get(message.id);
        if (!request) return;
        this.pending.delete(message.id);
        if (!message.ok) this.failuresInWorker++;
        if (this.pending.size === 0) this.idleWorker();
        if (message.ok) request.resolve(message.output);
        else request.reject(new EngineError(message.errorName, message.errorMessage));
    }

    /** Lets an idle worker stop the process from exiting, or replaces it if it has failed too often. */
    private idleWorker(): void {
        const worker = this.worker;
        if (!worker) return;
        if (this.failuresInWorker < this.failuresBeforeRestart) {
            worker.unref();
            return;
        }
        this.worker = undefined;
        void worker.terminate();
    }

    private failPending(message: string): void {
        const requests = [...this.pending.values()];
        this.pending.clear();
        for (const request of requests) request.reject(new EngineError(ENGINE_CRASH_ERROR, message));
    }
}
