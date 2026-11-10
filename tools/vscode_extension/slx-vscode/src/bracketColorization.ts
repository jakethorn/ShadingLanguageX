// slx.bracketPairColorization: whether matching brackets in SLX files are colour-coded by nesting level.
//
// It is off by default, as in the MaterialX Playground: language-configuration.json lists no
// colourised bracket pairs. The extension API can't change that list, but VS Code's
// editor.language.colorizedBracketPairs setting can, for one language. So this mirrors
// slx.bracketPairColorization into that setting for SLX, in the same settings file: on writes the
// bracket pairs, off writes none, and removing slx.bracketPairColorization removes what was written.
import * as vscode from 'vscode';

const BRACKET_PAIRS = [
    ['(', ')'],
    ['[', ']'],
    ['{', '}'],
];
const PAIRS_SETTING = 'language.colorizedBracketPairs';

/** The values this setting writes: the bracket pairs, or none. */
function isWrittenByThisSetting(value: unknown): boolean {
    const json = JSON.stringify(value);
    return json === JSON.stringify(BRACKET_PAIRS) || json === '[]';
}

async function applyBracketColorization(): Promise<void> {
    const setting = vscode.workspace.getConfiguration('slx').inspect<boolean>('bracketPairColorization');
    const editor = vscode.workspace.getConfiguration('editor', { languageId: 'slx' });
    const pairs = editor.inspect(PAIRS_SETTING);
    const levels = [
        { target: vscode.ConfigurationTarget.Global, on: setting?.globalValue, current: pairs?.globalLanguageValue },
        ...(vscode.workspace.workspaceFolders
            ? [{ target: vscode.ConfigurationTarget.Workspace, on: setting?.workspaceValue, current: pairs?.workspaceLanguageValue }]
            : []),
    ];
    for (const { target, on, current } of levels) {
        const wanted = on === undefined ? undefined : on ? BRACKET_PAIRS : [];
        if (JSON.stringify(current) === JSON.stringify(wanted)) continue;
        // Leave alone a value the user wrote themselves.
        if (current !== undefined && !isWrittenByThisSetting(current)) continue;
        await editor.update(PAIRS_SETTING, wanted, target, true);
    }
}

export function registerBracketColorization(log: vscode.LogOutputChannel): vscode.Disposable {
    const apply = () =>
        applyBracketColorization().catch((error) =>
            log.error(`Couldn't apply slx.bracketPairColorization: ${error instanceof Error ? error.message : String(error)}`),
        );
    void apply();
    return vscode.workspace.onDidChangeConfiguration((event) => {
        if (event.affectsConfiguration('slx.bracketPairColorization')) void apply();
    });
}
