// SLX: Install AI Skill. The skill bundled in skills/shadinglanguagex reaches VS Code's chat
// through the chatSkills contribution in package.json. Claude Code and other agents read skills
// from folders instead, so this copies it into one.
//
// A copy takes precedence over the bundled skill in VS Code's chat too, so the command also shows
// when a copy is out of date and offers to replace it.
import * as os from 'node:os';
import * as path from 'node:path';
import * as vscode from 'vscode';
import { compareSkill, copySkill, type SkillState } from './skillFiles';

const SKILL_NAME = 'shadinglanguagex';

interface SkillLocation {
    /** "personal", or the name of a workspace folder. */
    id: string;
    label: string;
    description: string;
    /** The folder the skill's own folder goes in. */
    skillsDir: string;
}

function findSkillLocations(): SkillLocation[] {
    const locations: SkillLocation[] = [
        {
            id: 'personal',
            label: 'Personal',
            description: '~/.claude/skills: for all your projects',
            skillsDir: path.join(os.homedir(), '.claude', 'skills'),
        },
    ];
    for (const folder of vscode.workspace.workspaceFolders ?? []) {
        if (folder.uri.scheme !== 'file') continue;
        locations.push({
            id: folder.name,
            label: `Workspace: ${folder.name}`,
            description: '.claude/skills: shared with everyone who uses the repository',
            skillsDir: path.join(folder.uri.fsPath, '.claude', 'skills'),
        });
    }
    return locations;
}

const STATE_DETAILS: Record<SkillState, string> = {
    missing: 'Not installed',
    current: 'Installed and up to date',
    different: 'Installed, but different from the version in this extension',
};

/**
 * Installs the skill. With a location ("personal", or a workspace folder's name), installs there
 * without asking where.
 */
async function installSkill(bundledDir: string, log: vscode.LogOutputChannel, locationId?: string): Promise<void> {
    const locations = findSkillLocations();
    let location: SkillLocation | undefined;
    if (locationId !== undefined) {
        location = locations.find((l) => l.id === locationId);
    } else {
        const picked = await vscode.window.showQuickPick(
            locations.map((l) => ({ ...l, detail: STATE_DETAILS[compareSkill(bundledDir, path.join(l.skillsDir, SKILL_NAME))] })),
            { title: 'Install the SLX Skill for AI Agents', placeHolder: 'Where should Claude Code and other agents find it?' },
        );
        location = picked;
    }
    if (!location) return;

    const target = path.join(location.skillsDir, SKILL_NAME);
    const state = compareSkill(bundledDir, target);
    if (state === 'current') {
        void vscode.window.showInformationMessage(`SLX: the skill in ${target} is already up to date.`);
        return;
    }
    if (state === 'different') {
        const replace = 'Replace';
        const choice = await vscode.window.showWarningMessage(
            `Replace the SLX skill in ${target}?`,
            { modal: true, detail: 'It is different from the version in this extension. Any changes made to it will be lost.' },
            replace,
        );
        if (choice !== replace) return;
    }

    try {
        copySkill(bundledDir, target);
    } catch (error) {
        const message = error instanceof Error ? error.message : String(error);
        log.error(`Couldn't install the SLX skill in ${target}: ${message}`);
        void vscode.window.showErrorMessage(`SLX: couldn't install the skill in ${target}.`);
        return;
    }
    log.info(`Installed the SLX skill in ${target}.`);
    const open = 'Open SKILL.md';
    void vscode.window
        .showInformationMessage(`SLX: installed the skill in ${target}. Agents pick it up in new sessions.`, open)
        .then((choice) => {
            if (choice === open) void vscode.window.showTextDocument(vscode.Uri.file(path.join(target, 'SKILL.md')));
        });
}

export function registerSkillCommand(context: vscode.ExtensionContext, log: vscode.LogOutputChannel): vscode.Disposable {
    const bundledDir = context.asAbsolutePath(path.join('skills', SKILL_NAME));
    return vscode.commands.registerCommand('slx.installSkill', (locationId?: string) => {
        if (compareSkill(bundledDir, bundledDir) === 'missing') {
            void vscode.window.showErrorMessage('SLX: the skill is not bundled with this build of the extension.');
            return;
        }
        return installSkill(bundledDir, log, locationId);
    });
}
