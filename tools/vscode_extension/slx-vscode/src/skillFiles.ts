// Compares and copies the SLX skill for AI agents: a folder holding SKILL.md and the files it refers to.
import * as fs from 'node:fs';
import * as path from 'node:path';

export type SkillState = 'missing' | 'current' | 'different';

/** Lists the files in a folder and its subfolders, by path relative to it, sorted. */
function listFiles(dir: string, prefix = ''): string[] {
    return fs
        .readdirSync(path.join(dir, prefix), { withFileTypes: true })
        .flatMap((entry) => {
            const relative = path.join(prefix, entry.name);
            return entry.isDirectory() ? listFiles(dir, relative) : [relative];
        })
        .sort();
}

/** Whether a skill folder is missing, the same as the bundled one, or different from it. */
export function compareSkill(bundledDir: string, installedDir: string): SkillState {
    if (!fs.existsSync(path.join(installedDir, 'SKILL.md'))) return 'missing';
    const bundled = listFiles(bundledDir);
    const installed = listFiles(installedDir);
    if (bundled.join('\n') !== installed.join('\n')) return 'different';
    const same = bundled.every((file) => fs.readFileSync(path.join(bundledDir, file)).equals(fs.readFileSync(path.join(installedDir, file))));
    return same ? 'current' : 'different';
}

/** Replaces the skill folder at installedDir with a copy of the bundled one. */
export function copySkill(bundledDir: string, installedDir: string): void {
    fs.rmSync(installedDir, { recursive: true, force: true });
    fs.mkdirSync(path.dirname(installedDir), { recursive: true });
    fs.cpSync(bundledDir, installedDir, { recursive: true });
}
