// Finds the files a document pulls in with #include and #library, so they can
// be handed to the compiler as in-memory sources.
//
// The compiler runs as WebAssembly with its own virtual file system, which
// only contains the MaterialX libraries. It cannot read the user's files, so
// the extension reads them and passes each one with CompileOptions.addSource.
//
// Each source is registered under the name (key) the compiler will look it up
// by. When the compiler resolves a directive, it first joins the path to the
// directory of the key of the source containing the directive, then falls
// back to the path as written. The document being compiled has no key, so its
// own directives are looked up as written. Keys therefore mirror the include
// tree: if the document includes "sub/a.mxsl" and that file includes
// "b.mxsl", the second file is registered as "sub/b.mxsl".
//
// Keys use POSIX path rules because the WebAssembly runtime does.
import * as fs from 'node:fs';
import * as path from 'node:path';

export type DirectiveKind = 'include' | 'library';

export interface Directive {
    kind: DirectiveKind;
    /** The path exactly as written between the quotes. */
    target: string;
    /** 0-based line of the directive. */
    line: number;
    /** 0-based column where the quoted path starts (at the opening quote). */
    start: number;
    /** 0-based column just after the closing quote. */
    end: number;
}

/** A directive, together with the source it was written in. */
export interface DirectiveOrigin {
    /** Key of the source containing the directive; ROOT_KEY for the document being compiled. */
    fromKey: string;
    /** File containing the directive, or undefined for an unsaved document. */
    fromFilePath: string | undefined;
    directive: Directive;
}

export interface IncludedSource {
    /** Name the compiler looks the source up by. */
    key: string;
    filePath: string;
    text: string;
    origin: DirectiveOrigin;
}

export interface SourceSet {
    sources: IncludedSource[];
    /** Directives whose file could not be found. Some may be in disabled #ifdef blocks. */
    missing: DirectiveOrigin[];
}

/** Returns the text of a file, or undefined if it does not exist or cannot be read. */
export type ReadSource = (filePath: string) => string | undefined;

export const ROOT_KEY = '';

// Guards against include trees that never end, such as a file including itself
// through a symbolic link to its own directory.
const MAX_INCLUDE_DEPTH = 64;
const MAX_SOURCES = 1000;

const DIRECTIVE = /^(\s*#\s*(include|library)\s+)("([^"]*)")/;

/** Replaces comments with spaces, keeping line breaks so positions stay the same. */
export function blankComments(text: string): string {
    const chars = text.split('');
    let i = 0;
    while (i < chars.length) {
        const c = chars[i];
        const next = chars[i + 1];
        if (c === '"') {
            i++;
            while (i < chars.length && chars[i] !== '"' && chars[i] !== '\n') i++;
            i++;
        } else if (c === '/' && next === '/') {
            while (i < chars.length && chars[i] !== '\n' && chars[i] !== '\r') chars[i++] = ' ';
        } else if (c === '/' && next === '*') {
            chars[i++] = ' ';
            chars[i++] = ' ';
            while (i < chars.length && !(chars[i] === '*' && chars[i + 1] === '/')) {
                if (chars[i] !== '\n' && chars[i] !== '\r') chars[i] = ' ';
                i++;
            }
            if (i < chars.length) {
                chars[i++] = ' ';
                chars[i++] = ' ';
            }
        } else {
            i++;
        }
    }
    return chars.join('');
}

export function findDirectives(text: string): Directive[] {
    const directives: Directive[] = [];
    blankComments(text)
        .split('\n')
        .forEach((lineText, line) => {
            const match = DIRECTIVE.exec(lineText);
            if (!match) return;
            const start = match[1].length;
            directives.push({
                kind: match[2] as DirectiveKind,
                target: match[4],
                line,
                start,
                end: start + match[3].length,
            });
        });
    return directives;
}

export function normaliseKey(name: string): string {
    const normal = path.posix.normalize(name);
    return normal === '.' ? '' : normal;
}

/** Returns the key the compiler looks up first for a directive in the source with key fromKey. */
export function joinKey(fromKey: string, target: string): string {
    const dir = path.posix.dirname(fromKey);
    const joined = target.startsWith('/') || dir === '.' ? target : `${dir}/${target}`;
    return normaliseKey(joined);
}

/** Returns the paths a directive's file is looked for at, in order. */
export function findCandidatePaths(target: string, fromFilePath: string | undefined, includePaths: readonly string[]): string[] {
    if (path.isAbsolute(target)) return [target];
    return [
        ...(fromFilePath ? [path.join(path.dirname(fromFilePath), target)] : []),
        ...includePaths.map((dir) => path.join(dir, target)),
    ];
}

function findSourceFile(
    target: string,
    fromFilePath: string | undefined,
    includePaths: readonly string[],
    read: ReadSource,
): { filePath: string; text: string } | undefined {
    for (const candidate of findCandidatePaths(target, fromFilePath, includePaths)) {
        const text = read(candidate);
        if (text !== undefined) return { filePath: path.resolve(candidate), text };
    }
    return undefined;
}

/**
 * Returns the keys to register a source under. Libraries are also registered
 * under the path as written: the compiler resolves them after preprocessing,
 * against whichever directory it last included from, so the joined key alone
 * is not always the one it tries.
 */
function keysForDirective(fromKey: string, directive: Directive): string[] {
    const joined = joinKey(fromKey, directive.target);
    if (directive.kind === 'include') return [joined];
    const asWritten = normaliseKey(directive.target);
    return joined === asWritten ? [joined] : [joined, asWritten];
}

/**
 * Finds every file reachable through #include and #library directives from
 * the document. Directives inside disabled #ifdef blocks are followed too:
 * reading a file the compiler ends up not needing is harmless.
 */
export function collectSources(
    rootFilePath: string | undefined,
    rootText: string,
    includePaths: readonly string[],
    read: ReadSource,
): SourceSet {
    const sources: IncludedSource[] = [];
    const missing: DirectiveOrigin[] = [];
    const takenKeys = new Set<string>();

    const visit = (fromKey: string, fromFilePath: string | undefined, text: string, depth: number) => {
        if (depth > MAX_INCLUDE_DEPTH) return;
        for (const directive of findDirectives(text)) {
            if (sources.length >= MAX_SOURCES) return;
            const origin: DirectiveOrigin = { fromKey, fromFilePath, directive };
            const found = findSourceFile(directive.target, fromFilePath, includePaths, read);
            if (!found) {
                missing.push(origin);
                continue;
            }
            const newKeys = keysForDirective(fromKey, directive).filter((key) => !takenKeys.has(key));
            for (const key of newKeys) {
                takenKeys.add(key);
                sources.push({ key, filePath: found.filePath, text: found.text, origin });
            }
            if (directive.kind === 'include' && newKeys.length > 0) {
                visit(newKeys[0], found.filePath, found.text, depth + 1);
            }
        }
    };

    visit(ROOT_KEY, rootFilePath, rootText, 0);
    return { sources, missing };
}

export interface FolderEntry {
    name: string;
    isFolder: boolean;
}

/**
 * Lists what a directive's path can continue with: the subfolders, and the files with the extension, in each of
 * the folders. Hidden entries are left out, and a name is listed once even if several folders have it.
 */
export function listDirectiveEntries(folders: readonly string[], extension: string, exclude?: string): FolderEntry[] {
    const entries = new Map<string, FolderEntry>();
    for (const folder of folders) {
        let dirents: fs.Dirent[];
        try {
            dirents = fs.readdirSync(folder, { withFileTypes: true });
        } catch {
            continue;
        }
        for (const dirent of dirents) {
            if (dirent.name.startsWith('.') || entries.has(dirent.name)) continue;
            const fullPath = path.join(folder, dirent.name);
            let isFolder = dirent.isDirectory();
            if (dirent.isSymbolicLink()) {
                try {
                    isFolder = fs.statSync(fullPath).isDirectory();
                } catch {
                    continue;
                }
            }
            const matches = isFolder || path.extname(dirent.name).toLowerCase() === extension;
            if (matches && path.resolve(fullPath) !== exclude) entries.set(dirent.name, { name: dirent.name, isFolder });
        }
    }
    return [...entries.values()];
}
