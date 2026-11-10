// Reads the location out of an mxslc error message.
//
// The compiler reports errors as plain text, prefixed with the file and line
// when it knows them:
//
//     line 2: Variable or parameterless function not defined: x
//     colors.mxsl, line 4: Invalid expression
//     Scanning error on line 3, character: $
//     File colors.mxsl could not be found.\nSearched paths:\n...
//
// The file is only ever a file name, never a path, and is absent for errors in
// the source the compiler was given directly. Lines are 1-based.
import * as path from 'node:path';
import { ROOT_KEY, normaliseKey, type DirectiveOrigin, type IncludedSource, type SourceSet } from './includes';

export interface CompileErrorLocation {
    /** File name the error is in, or undefined for the compiled source itself. */
    fileName?: string;
    /** 1-based line number, or undefined when the compiler gave none. */
    line?: number;
    /** The error text without its location prefix. */
    message: string;
}

const LOCATED_ERROR = /^(?:(.+?), )?line (\d+): ([\s\S]*)$/;
const SCANNING_ERROR = /^Scanning error on line (\d+)/;
const MISSING_FILE_ERROR = /^File (.+?) could not be found\./;
const ANSI_ESCAPE = /\x1b\[[0-9;]*m/g;

export function stripAnsiCodes(text: string): string {
    return text.replace(ANSI_ESCAPE, '');
}

export function parseCompileError(text: string): CompileErrorLocation {
    const message = stripAnsiCodes(text).trim();

    const located = LOCATED_ERROR.exec(message);
    if (located) {
        return {
            fileName: located[1],
            line: Number(located[2]),
            message: located[3].trim(),
        };
    }

    const scanning = SCANNING_ERROR.exec(message);
    if (scanning) {
        return { line: Number(scanning[1]), message };
    }

    return { message };
}

/** Returns the path in a "File X could not be found" error, if it is one. */
export function findMissingFile(message: string): string | undefined {
    return MISSING_FILE_ERROR.exec(message)?.[1];
}

const CHARACTER_OFFSET = /at character (\d+)/;

/** Returns the character offset in a MaterialX XML parse error, if it has one. */
export function findCharacterOffset(message: string): number | undefined {
    const match = CHARACTER_OFFSET.exec(message);
    return match ? Number(match[1]) : undefined;
}

/** A range on one line, in the document being compiled or in a file it includes. */
export interface ErrorSpan {
    /** The included file the span is in, or undefined for the document being compiled. */
    filePath: string | undefined;
    /** 0-based line. */
    line: number;
    /** 0-based start and end columns. */
    start: number;
    end: number;
}

export interface ErrorReport {
    span: ErrorSpan;
    message: string;
    /** For an error reported on an #include line: where the error actually is. */
    related?: { span: ErrorSpan; message: string };
}

/** Returns the span covering a line's text, ignoring leading and trailing whitespace. */
function spanLine(filePath: string | undefined, text: string, line: number): ErrorSpan {
    const lines = text.split('\n');
    const index = Math.max(0, Math.min(line, lines.length - 1));
    const lineText = lines[index].replace(/\s+$/, '');
    const start = lineText.length - lineText.trimStart().length;
    return { filePath, line: index, start, end: lineText.length };
}

function spanDirective(origin: DirectiveOrigin): ErrorSpan {
    const { line, start, end } = origin.directive;
    return { filePath: origin.fromKey === ROOT_KEY ? undefined : origin.fromFilePath, line, start, end };
}

function findSourceByKey(sourceSet: SourceSet, key: string): IncludedSource | undefined {
    return sourceSet.sources.find((source) => source.key === key);
}

function findSourceByFileName(sourceSet: SourceSet, fileName: string): IncludedSource | undefined {
    return sourceSet.sources.find((source) => path.posix.basename(source.key) === fileName);
}

function findMissingDirective(sourceSet: SourceSet, missingPath: string): DirectiveOrigin | undefined {
    const wanted = normaliseKey(missingPath);
    return (
        sourceSet.missing.find((origin) => normaliseKey(origin.directive.target) === wanted) ??
        sourceSet.missing.find((origin) => path.posix.basename(origin.directive.target) === path.posix.basename(wanted))
    );
}

/** Follows the include chain up to the directive in the document being compiled. */
function findRootDirective(sourceSet: SourceSet, origin: DirectiveOrigin): DirectiveOrigin {
    let current = origin;
    for (let depth = 0; current.fromKey !== ROOT_KEY && depth < sourceSet.sources.length; depth++) {
        const parent = findSourceByKey(sourceSet, current.fromKey);
        if (!parent) break;
        current = parent.origin;
    }
    return current;
}

/** Reports an error in an included file on the #include line of the document being compiled. */
function reportOnRootDirective(sourceSet: SourceSet, origin: DirectiveOrigin, includedName: string, error: ErrorReport): ErrorReport {
    return {
        span: spanDirective(findRootDirective(sourceSet, origin)),
        message: `Error in included file "${includedName}": ${error.message}`,
        related: { span: error.span, message: error.message },
    };
}

/**
 * Works out where a compile error is. An error in an included file is
 * reported in that file and again on the #include line of the document being
 * compiled, so it is visible from the document.
 */
export function locateCompileError(errorMessage: string, rootText: string, sourceSet: SourceSet): ErrorReport[] {
    const location = parseCompileError(errorMessage);

    if (location.fileName !== undefined) {
        const source = findSourceByFileName(sourceSet, location.fileName);
        if (!source) {
            return [{ span: spanLine(undefined, rootText, 0), message: `${location.fileName}, line ${location.line}: ${location.message}` }];
        }
        const error = { span: spanLine(source.filePath, source.text, (location.line ?? 1) - 1), message: location.message };
        return [error, reportOnRootDirective(sourceSet, source.origin, source.key, error)];
    }

    const missingPath = findMissingFile(location.message);
    const missing = missingPath !== undefined ? findMissingDirective(sourceSet, missingPath) : undefined;
    if (missing) {
        const error = {
            span: spanDirective(missing),
            message: `Cannot find "${missing.directive.target}". Files are looked for next to the file containing the directive, then in the slx.includePaths folders.`,
        };
        const includer = findSourceByKey(sourceSet, missing.fromKey);
        return includer ? [error, reportOnRootDirective(sourceSet, includer.origin, includer.key, error)] : [error];
    }

    const line = location.line !== undefined ? location.line - 1 : 0;
    return [{ span: spanLine(undefined, rootText, line), message: location.message }];
}

/** Works out where a decompile error is in the MaterialX document. */
export function locateDecompileError(errorMessage: string, text: string): ErrorReport {
    const message = stripAnsiCodes(errorMessage).trim();
    const offset = findCharacterOffset(message);
    const line = offset !== undefined ? text.slice(0, offset).split('\n').length - 1 : 0;
    return { span: spanLine(undefined, text, line), message };
}
