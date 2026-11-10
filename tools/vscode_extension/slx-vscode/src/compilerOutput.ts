// Shows text the compiler prints in the SLX output channel.
import type * as vscode from 'vscode';
import { stripAnsiCodes } from './compileErrors';
import type { EngineLogger } from './engine';

/** For commands the user runs: shows `print` output and warnings. */
export function createCommandLogger(channel: vscode.LogOutputChannel): EngineLogger {
    return (stream, text) => {
        const line = stripAnsiCodes(text);
        if (stream === 'stderr') channel.warn(line);
        else if (line.length > 0) channel.info(`print: ${line}`);
    };
}

/**
 * For checks run as files are edited: shows warnings, skipping a warning that
 * repeats the previous one, and leaves out `print` output, which would
 * otherwise be shown again on every keystroke.
 */
export function createBackgroundLogger(channel: vscode.LogOutputChannel): EngineLogger {
    let previousWarning: string | undefined;
    return (stream, text) => {
        if (stream !== 'stderr') return;
        const line = stripAnsiCodes(text);
        if (line === previousWarning) return;
        previousWarning = line;
        channel.warn(line);
    };
}
