// The status bar item for SLX files: the MaterialX version and whether the file compiles.
// Clicking it runs SLX: Select MaterialX Version.
import * as vscode from 'vscode';
import type { CheckEvent, SlxDiagnostics } from './diagnostics';
import { isSlxDocument } from './documents';
import { findVersionDirective, type SlxLanguage } from './languageData';
import { readSettings } from './settings';

type CompileState = 'ok' | 'errors' | 'unavailable';

const ICONS: Record<CompileState, string> = { ok: '$(check)', errors: '$(error)', unavailable: '$(warning)' };
const DESCRIPTIONS: Record<CompileState, string> = {
    ok: 'The file compiles.',
    errors: 'The file has errors. See the Problems panel.',
    unavailable: 'The SLX compiler is unavailable. See the SLX output panel.',
};

export class SlxStatusBar implements vscode.Disposable {
    private readonly item = vscode.window.createStatusBarItem('slx.status', vscode.StatusBarAlignment.Right, 100);
    private readonly states = new Map<string, CompileState>();
    private readonly disposables: vscode.Disposable[];

    constructor(
        diagnostics: SlxDiagnostics,
        private readonly language: SlxLanguage | undefined,
        private readonly packagedVersions: string[],
    ) {
        this.item.name = 'SLX';
        this.item.command = 'slx.selectMaterialXVersion';
        this.disposables = [
            this.item,
            diagnostics.onDidCheck((event) => this.record(event)),
            vscode.window.onDidChangeActiveTextEditor(() => this.refresh()),
            vscode.workspace.onDidChangeTextDocument((event) => {
                if (event.document === vscode.window.activeTextEditor?.document) this.refresh();
            }),
            vscode.workspace.onDidChangeConfiguration((event) => {
                if (event.affectsConfiguration('slx')) this.refresh();
            }),
            vscode.workspace.onDidCloseTextDocument((document) => this.states.delete(document.uri.toString())),
            vscode.commands.registerCommand('slx.selectMaterialXVersion', () => this.selectVersion()),
        ];
        this.refresh();
    }

    dispose(): void {
        for (const disposable of this.disposables) disposable.dispose();
    }

    private record({ document, result }: CheckEvent): void {
        const state: CompileState = result.output !== undefined ? 'ok' : result.error?.isEngineFailure ? 'unavailable' : 'errors';
        this.states.set(document.uri.toString(), state);
        if (document === vscode.window.activeTextEditor?.document) this.refresh();
    }

    private refresh(): void {
        const document = vscode.window.activeTextEditor?.document;
        if (!document || !isSlxDocument(document)) {
            this.item.hide();
            return;
        }
        const directive = findVersionDirective(document.getText());
        const version = directive ?? readSettings(document.uri).compile.materialxVersion;
        const state = this.states.get(document.uri.toString());
        this.item.text = `${state ? `${ICONS[state]} ` : ''}SLX ${version}`;
        const source = directive ? 'set by the `#version` directive in this file' : 'from the `slx.materialxVersion` setting';
        this.item.tooltip = new vscode.MarkdownString(
            [`MaterialX ${version}, ${source}.`, state ? DESCRIPTIONS[state] : '', 'Click to choose the MaterialX version.']
                .filter((line) => line !== '')
                .join('\n\n'),
        );
        this.item.show();
    }

    private async selectVersion(): Promise<void> {
        const document = vscode.window.activeTextEditor?.document;
        const uri = document && isSlxDocument(document) ? document.uri : undefined;
        const current = readSettings(uri).compile.materialxVersion;
        const directive = document && uri ? findVersionDirective(document.getText()) : undefined;
        const versions = this.language?.versions ?? this.packagedVersions;
        const picked = await vscode.window.showQuickPick(
            [...versions].reverse().map((version) => ({ label: version, description: version === current ? 'current setting' : undefined })),
            {
                title: 'MaterialX Version',
                placeHolder: directive
                    ? `This file's #version ${directive} directive takes precedence over the setting`
                    : 'The MaterialX version to compile SLX files against',
            },
        );
        if (!picked || picked.label === current) return;
        const target = vscode.workspace.workspaceFolders ? vscode.ConfigurationTarget.Workspace : vscode.ConfigurationTarget.Global;
        await vscode.workspace.getConfiguration('slx', uri).update('materialxVersion', picked.label, target);
    }
}
