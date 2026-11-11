import assert from 'node:assert/strict';
import * as fs from 'node:fs';
import * as os from 'node:os';
import * as path from 'node:path';
import { test } from 'node:test';
import { compareSkill, copySkill } from '../src/skillFiles';

function makeSkill(files: Record<string, string>): string {
    const dir = fs.mkdtempSync(path.join(os.tmpdir(), 'slx-skill-'));
    for (const [name, text] of Object.entries(files)) {
        fs.mkdirSync(path.dirname(path.join(dir, name)), { recursive: true });
        fs.writeFileSync(path.join(dir, name), text);
    }
    return dir;
}

const bundled = makeSkill({ 'SKILL.md': '---\nname: shadinglanguagex\n---\n', 'references/language.md': 'language' });

test('compareSkill tells a missing, current and different copy apart', () => {
    const parent = fs.mkdtempSync(path.join(os.tmpdir(), 'slx-skills-'));
    assert.equal(compareSkill(bundled, path.join(parent, 'shadinglanguagex')), 'missing');
    assert.equal(compareSkill(bundled, makeSkill({ 'SKILL.md': '---\nname: shadinglanguagex\n---\n', 'references/language.md': 'language' })), 'current');
    assert.equal(compareSkill(bundled, makeSkill({ 'SKILL.md': '---\nname: shadinglanguagex\n---\n', 'references/language.md': 'older' })), 'different');
    assert.equal(compareSkill(bundled, makeSkill({ 'SKILL.md': '---\nname: shadinglanguagex\n---\n' })), 'different');
});

test('copySkill installs the skill, replacing an older copy completely', () => {
    const parent = fs.mkdtempSync(path.join(os.tmpdir(), 'slx-skills-'));
    const target = path.join(parent, 'nested', 'shadinglanguagex');
    copySkill(bundled, target);
    assert.equal(compareSkill(bundled, target), 'current');
    fs.writeFileSync(path.join(target, 'references', 'language.md'), 'edited');
    fs.writeFileSync(path.join(target, 'extra.md'), 'extra');
    copySkill(bundled, target);
    assert.equal(compareSkill(bundled, target), 'current');
});
