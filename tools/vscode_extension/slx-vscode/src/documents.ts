// Helpers for the documents the extension works with.
import * as fs from 'node:fs';
import * as path from 'node:path';
import * as vscode from 'vscode';
import { collectSources, type SourceSet } from './includes';

export function isSlxDocument(document: vscode.TextDocument): boolean {
    return document.languageId === 'slx';
}

export function isMaterialXDocument(document: vscode.TextDocument): boolean {
    return path.extname(document.fileName).toLowerCase() === '.mtlx';
}

/** Returns a path that compares equal for the same file, ignoring case on Windows. */
export function toComparablePath(filePath: string): string {
    const resolved = path.resolve(filePath);
    return process.platform === 'win32' ? resolved.toLowerCase() : resolved;
}

/** Reads a file from its open editor if there is one, so unsaved changes are used, otherwise from disk. */
function readFromEditorOrDisk(filePath: string): string | undefined {
    const wanted = toComparablePath(filePath);
    const open = vscode.workspace.textDocuments.find(
        (document) => document.uri.scheme === 'file' && toComparablePath(document.uri.fsPath) === wanted,
    );
    if (open) return open.getText();
    try {
        return fs.statSync(filePath).isFile() ? fs.readFileSync(filePath, 'utf8') : undefined;
    } catch {
        return undefined;
    }
}

/** Finds the files the document pulls in with #include and #library. */
export function collectDocumentSources(
    document: vscode.TextDocument,
    text: string,
    includePaths: readonly string[],
): SourceSet {
    const filePath = document.uri.scheme === 'file' ? document.uri.fsPath : undefined;
    return collectSources(filePath, text, includePaths, readFromEditorOrDisk);
}
