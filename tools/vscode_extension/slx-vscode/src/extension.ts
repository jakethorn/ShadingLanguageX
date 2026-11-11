// Entry point of the SLX extension.
import * as vscode from 'vscode';
import { registerBracketColorization } from './bracketColorization';
import { registerCommands } from './commands';
import { COMPLETION_TRIGGER_CHARACTERS, SlxCompletionProvider } from './completion';
import { SlxDiagnostics } from './diagnostics';
import { SlxEngine } from './engine';
import { SlxHoverProvider } from './hover';
import { SlxLanguage } from './languageData';
import { SlxDefinitionProvider, SlxDocumentLinkProvider } from './navigation';
import { SlxDocumentSymbolProvider } from './outline';
import { MaterialXPreview } from './preview';
import { SEMANTIC_TOKENS_LEGEND, SlxSemanticTokensProvider } from './semanticTokens';
import { SlxSignatureHelpProvider } from './signatureHelp';
import { registerSkillCommand } from './skill';
import { SlxStatusBar } from './statusBar';
import { SymbolIndex } from './symbolIndex';

const SLX: vscode.DocumentSelector = { language: 'slx' };

export function activate(context: vscode.ExtensionContext): void {
    const log = vscode.window.createOutputChannel('SLX', { log: true });

    const engine = new SlxEngine(context.asAbsolutePath('dist/engineWorker.mjs'), context.asAbsolutePath('media/lib'));
    if (!engine.isBundled) {
        log.warn(
            'The SLX compiler is not bundled with this build of the extension. Highlighting and snippets work, ' +
                'but compiling, validating and live error checking are unavailable.',
        );
    }
    const language = SlxLanguage.load(context.asAbsolutePath('data'));
    if (!language) {
        log.warn('The SLX language data is missing from this build of the extension, so MaterialX nodes, types and keywords are not completed or described.');
    }

    const diagnostics = new SlxDiagnostics(engine, log);
    const index = new SymbolIndex(language);
    const semanticTokens = new SlxSemanticTokensProvider(index);
    const packagedVersions: string[] = context.extension.packageJSON.contributes.configuration.properties['slx.materialxVersion'].enum;
    context.subscriptions.push(
        log,
        diagnostics,
        { dispose: () => engine.dispose() },
        ...registerCommands(diagnostics, context.workspaceState, log),
        index,
        vscode.languages.registerCompletionItemProvider(SLX, new SlxCompletionProvider(index), ...COMPLETION_TRIGGER_CHARACTERS),
        vscode.languages.registerHoverProvider(SLX, new SlxHoverProvider(index)),
        vscode.languages.registerSignatureHelpProvider(SLX, new SlxSignatureHelpProvider(index), '(', ','),
        vscode.languages.registerDocumentSymbolProvider(SLX, new SlxDocumentSymbolProvider(index)),
        vscode.languages.registerDefinitionProvider(SLX, new SlxDefinitionProvider(index)),
        vscode.languages.registerDocumentLinkProvider(SLX, new SlxDocumentLinkProvider(index)),
        semanticTokens,
        vscode.languages.registerDocumentSemanticTokensProvider(SLX, semanticTokens, SEMANTIC_TOKENS_LEGEND),
        new MaterialXPreview(diagnostics),
        new SlxStatusBar(diagnostics, language, packagedVersions),
        registerBracketColorization(log),
        registerSkillCommand(context, log),
    );
}

export function deactivate(): void {}
