// Reads the slx.* settings.
import * as path from 'node:path';
import * as vscode from 'vscode';
import type { CompileSettings, FunctionBehaviour } from './engineProtocol';

export type DiagnosticsTrigger = 'onType' | 'onSave' | 'off';

export interface SlxSettings {
    compile: CompileSettings;
    /** Absolute folders searched for #include and #library files. */
    includePaths: string[];
    diagnosticsTrigger: DiagnosticsTrigger;
    /** Make MaterialX node names links to their section of the specification. */
    nodeLinks: boolean;
}

function resolveIncludePath(includePath: string, folder: vscode.WorkspaceFolder | undefined): string {
    const expanded = folder ? includePath.replace(/\$\{workspaceFolder\}/g, folder.uri.fsPath) : includePath;
    if (path.isAbsolute(expanded) || !folder) return expanded;
    return path.join(folder.uri.fsPath, expanded);
}

/** Reads the settings that apply to the given document, or the workspace settings if there is none. */
export function readSettings(uri: vscode.Uri | undefined): SlxSettings {
    const config = vscode.workspace.getConfiguration('slx', uri);
    const folder = (uri && vscode.workspace.getWorkspaceFolder(uri)) ?? vscode.workspace.workspaceFolders?.[0];
    return {
        compile: {
            materialxVersion: config.get<string>('materialxVersion', '1.39.5'),
            reduceGraph: config.get<boolean>('reduceGraph', true),
            defaultFunctionBehaviour: config.get<FunctionBehaviour>('defaultFunctionBehaviour', 'nodedef'),
            errorOnMissingGlobals: config.get<boolean>('errorOnMissingGlobals', false),
        },
        includePaths: config.get<string[]>('includePaths', []).map((p) => resolveIncludePath(p, folder)),
        diagnosticsTrigger: config.get<DiagnosticsTrigger>('diagnostics.trigger', 'onType'),
        nodeLinks: config.get<boolean>('links.nodes', false),
    };
}
