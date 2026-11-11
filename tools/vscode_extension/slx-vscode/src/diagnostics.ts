// Compiles open SLX documents and shows their errors in the Problems panel.
//
// Each check belongs to the document that was compiled (its owner), but can
// put errors in other files: an error in an included file is shown in that
// file as well as on the document's #include line. The diagnostics of all
// owners are merged per file.
import * as vscode from 'vscode';
import { locateCompileError, locateDecompileError, type ErrorReport, type ErrorSpan } from './compileErrors';
import { createBackgroundLogger } from './compilerOutput';
import { collectDocumentSources, isSlxDocument, toComparablePath } from './documents';
import { EngineError, type EngineLogger, type SlxEngine } from './engine';
import { readSettings, type DiagnosticsTrigger } from './settings';

/** How long to wait after the last keystroke before checking a document. */
const TYPING_DELAY_MS = 400;

export interface CheckResult {
    output?: string;
    error?: EngineError;
}

export interface CheckEvent {
    document: vscode.TextDocument;
    result: CheckResult;
}

interface FileDiagnostics {
    uri: vscode.Uri;
    diagnostics: vscode.Diagnostic[];
}

function toEngineError(error: unknown): EngineError {
    if (error instanceof EngineError) return error;
    return new EngineError('Error', error instanceof Error ? error.message : String(error));
}

function isCheckable(document: vscode.TextDocument): boolean {
    return isSlxDocument(document) && (document.uri.scheme === 'file' || document.uri.scheme === 'untitled');
}

export class SlxDiagnostics implements vscode.Disposable {
    private readonly collection = vscode.languages.createDiagnosticCollection('slx');
    /** Diagnostics from each owner's last check, by file. */
    private readonly diagnosticsByOwner = new Map<string, Map<string, FileDiagnostics>>();
    /** Files each owner's last check read through #include and #library. */
    private readonly dependenciesByOwner = new Map<string, Set<string>>();
    private readonly timers = new Map<string, NodeJS.Timeout>();
    /** Owners with a check running, and whether another check was asked for meanwhile. */
    private readonly running = new Map<string, boolean>();
    private readonly disposables: vscode.Disposable[] = [];
    private readonly backgroundLog: EngineLogger;
    private lastEngineFailure: string | undefined;
    /** Documents checked as they are typed whatever slx.diagnostics.trigger says, because a preview shows them. */
    private readonly keptUpdated = new Set<string>();
    private readonly checked = new vscode.EventEmitter<CheckEvent>();
    /** Fired after each compile of an SLX document. */
    readonly onDidCheck = this.checked.event;

    constructor(
        private readonly engine: SlxEngine,
        private readonly log: vscode.LogOutputChannel,
    ) {
        this.backgroundLog = createBackgroundLogger(log);
        this.disposables.push(
            this.collection,
            this.checked,
            vscode.workspace.onDidOpenTextDocument((document) => this.handleOpen(document)),
            vscode.workspace.onDidChangeTextDocument((event) => this.handleChange(event)),
            vscode.workspace.onDidSaveTextDocument((document) => this.handleSave(document)),
            vscode.workspace.onDidCloseTextDocument((document) => this.handleClose(document)),
            vscode.workspace.onDidChangeConfiguration((event) => {
                if (event.affectsConfiguration('slx')) this.checkOpenDocuments();
            }),
        );
        this.checkOpenDocuments();
    }

    dispose(): void {
        for (const timer of this.timers.values()) clearTimeout(timer);
        this.timers.clear();
        for (const disposable of this.disposables) disposable.dispose();
    }

    /** Checks the document as it is typed while on is true, whatever the slx.diagnostics.trigger setting says. */
    keepUpdated(uri: vscode.Uri, on: boolean): void {
        if (on) this.keptUpdated.add(uri.toString());
        else this.keptUpdated.delete(uri.toString());
    }

    /** Compiles an SLX document now and updates its diagnostics. Text the compiler prints goes to compilerLog. */
    async check(document: vscode.TextDocument, compilerLog: EngineLogger = this.backgroundLog): Promise<CheckResult> {
        const owner = document.uri.toString();
        const text = document.getText();
        const settings = readSettings(document.uri);
        const sourceSet = collectDocumentSources(document, text, settings.includePaths);
        this.dependenciesByOwner.set(owner, new Set(sourceSet.sources.map((source) => toComparablePath(source.filePath))));

        let result: CheckResult;
        try {
            const sources = sourceSet.sources.map(({ key, text }) => ({ key, text }));
            result = { output: await this.engine.compile(text, sources, settings.compile, compilerLog) };
        } catch (error) {
            result = { error: toEngineError(error) };
        }

        if (document.isClosed) return result;
        if (result.error?.isEngineFailure) {
            this.reportEngineFailure(result.error);
            this.setOwnerDiagnostics(owner, []);
        } else {
            const reports = result.error ? locateCompileError(result.error.message, text, sourceSet) : [];
            this.setOwnerDiagnostics(owner, reports.map((report) => this.toFileDiagnostics(document.uri, report)));
        }
        this.checked.fire({ document, result });
        return result;
    }

    /** Decompiles a MaterialX document now and updates its diagnostics. Text the decompiler prints goes to compilerLog. */
    async checkMaterialX(document: vscode.TextDocument, compilerLog: EngineLogger = this.backgroundLog): Promise<CheckResult> {
        const owner = document.uri.toString();
        const text = document.getText();
        let result: CheckResult;
        try {
            result = { output: await this.engine.decompile(text, compilerLog) };
        } catch (error) {
            result = { error: toEngineError(error) };
        }

        if (document.isClosed) return result;
        if (result.error && !result.error.isEngineFailure) {
            this.setOwnerDiagnostics(owner, [this.toFileDiagnostics(document.uri, locateDecompileError(result.error.message, text))]);
        } else {
            this.setOwnerDiagnostics(owner, []);
        }
        return result;
    }

    private triggerFor(document: vscode.TextDocument): DiagnosticsTrigger {
        return this.keptUpdated.has(document.uri.toString()) ? 'onType' : readSettings(document.uri).diagnosticsTrigger;
    }

    private handleOpen(document: vscode.TextDocument): void {
        if (isCheckable(document) && this.triggerFor(document) !== 'off') this.schedule(document, 0);
    }

    private handleChange(event: vscode.TextDocumentChangeEvent): void {
        // Also fired when only the dirty state changes.
        if (event.contentChanges.length === 0) return;
        const document = event.document;
        if (isCheckable(document) && this.triggerFor(document) === 'onType') this.schedule(document, TYPING_DELAY_MS);
        this.scheduleDependents(document, 'type');
    }

    private handleSave(document: vscode.TextDocument): void {
        if (isCheckable(document) && this.triggerFor(document) !== 'off') this.schedule(document, 0);
        this.scheduleDependents(document, 'save');
    }

    private handleClose(document: vscode.TextDocument): void {
        const owner = document.uri.toString();
        clearTimeout(this.timers.get(owner));
        this.timers.delete(owner);
        this.dependenciesByOwner.delete(owner);
        this.setOwnerDiagnostics(owner, []);
        this.diagnosticsByOwner.delete(owner);
    }

    private checkOpenDocuments(): void {
        for (const document of vscode.workspace.textDocuments) {
            if (!isCheckable(document)) continue;
            if (this.triggerFor(document) === 'off') this.setOwnerDiagnostics(document.uri.toString(), []);
            else this.schedule(document, 0);
        }
    }

    /** Rechecks the open documents that include a file that just changed. */
    private scheduleDependents(changed: vscode.TextDocument, reason: 'type' | 'save'): void {
        if (changed.uri.scheme !== 'file') return;
        const changedPath = toComparablePath(changed.uri.fsPath);
        for (const [owner, dependencies] of this.dependenciesByOwner) {
            if (owner === changed.uri.toString() || !dependencies.has(changedPath)) continue;
            const document = vscode.workspace.textDocuments.find((d) => d.uri.toString() === owner);
            if (!document) continue;
            const trigger = this.triggerFor(document);
            if (trigger === 'onType') this.schedule(document, reason === 'type' ? TYPING_DELAY_MS : 0);
            else if (trigger === 'onSave' && reason === 'save') this.schedule(document, 0);
        }
    }

    private schedule(document: vscode.TextDocument, delayMs: number): void {
        if (!this.engine.isBundled) return;
        const owner = document.uri.toString();
        clearTimeout(this.timers.get(owner));
        this.timers.set(
            owner,
            setTimeout(() => {
                this.timers.delete(owner);
                void this.runScheduledCheck(document);
            }, delayMs),
        );
    }

    /** Runs a check, or if one is already running for the document, runs another once it finishes. */
    private async runScheduledCheck(document: vscode.TextDocument): Promise<void> {
        if (document.isClosed) return;
        const owner = document.uri.toString();
        if (this.running.has(owner)) {
            this.running.set(owner, true);
            return;
        }
        this.running.set(owner, false);
        try {
            await this.check(document);
        } catch (error) {
            this.log.error(`Checking ${document.uri.fsPath} failed: ${toEngineError(error).message}`);
        } finally {
            const rerun = this.running.get(owner);
            this.running.delete(owner);
            if (rerun) void this.runScheduledCheck(document);
        }
    }

    private reportEngineFailure(error: EngineError): void {
        if (this.lastEngineFailure === error.message) return;
        this.lastEngineFailure = error.message;
        this.log.error(error.message);
    }

    private toRange(span: ErrorSpan): vscode.Range {
        return new vscode.Range(span.line, span.start, span.line, span.end);
    }

    private toUri(ownerUri: vscode.Uri, span: ErrorSpan): vscode.Uri {
        return span.filePath === undefined ? ownerUri : vscode.Uri.file(span.filePath);
    }

    private toFileDiagnostics(ownerUri: vscode.Uri, report: ErrorReport): FileDiagnostics {
        const diagnostic = new vscode.Diagnostic(this.toRange(report.span), report.message, vscode.DiagnosticSeverity.Error);
        diagnostic.source = 'slx';
        if (report.related) {
            const location = new vscode.Location(this.toUri(ownerUri, report.related.span), this.toRange(report.related.span));
            diagnostic.relatedInformation = [new vscode.DiagnosticRelatedInformation(location, report.related.message)];
        }
        return { uri: this.toUri(ownerUri, report.span), diagnostics: [diagnostic] };
    }

    private setOwnerDiagnostics(owner: string, files: FileDiagnostics[]): void {
        const byFile = new Map<string, FileDiagnostics>();
        for (const file of files) {
            const key = file.uri.toString();
            const existing = byFile.get(key);
            if (existing) existing.diagnostics.push(...file.diagnostics);
            else byFile.set(key, { uri: file.uri, diagnostics: [...file.diagnostics] });
        }

        const previous = this.diagnosticsByOwner.get(owner);
        this.diagnosticsByOwner.set(owner, byFile);
        const affected = new Map<string, vscode.Uri>();
        for (const file of [...(previous?.values() ?? []), ...byFile.values()]) affected.set(file.uri.toString(), file.uri);
        for (const [key, uri] of affected) this.refreshFile(key, uri);
    }

    /** Shows the merged diagnostics of every owner for one file, without duplicates. */
    private refreshFile(key: string, uri: vscode.Uri): void {
        const merged: vscode.Diagnostic[] = [];
        const seen = new Set<string>();
        for (const byFile of this.diagnosticsByOwner.values()) {
            for (const diagnostic of byFile.get(key)?.diagnostics ?? []) {
                const id = `${diagnostic.range.start.line}:${diagnostic.message}`;
                if (seen.has(id)) continue;
                seen.add(id);
                merged.push(diagnostic);
            }
        }
        if (merged.length > 0) this.collection.set(uri, merged);
        else this.collection.delete(uri);
    }
}
