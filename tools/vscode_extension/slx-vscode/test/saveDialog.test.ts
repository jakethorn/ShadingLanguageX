import assert from 'node:assert/strict';
import { test } from 'node:test';
import { toSaveDialogDefault } from '../src/saveDialog';

const unc = { scheme: 'file', authority: 'wsl.localhost', path: '/Ubuntu/home/jaket/examples/vstext.mtlx', query: '', fragment: '' };

test('toSaveDialogDefault moves the server of a UNC path into the path on local Windows', () => {
    assert.deepEqual(toSaveDialogDefault(unc, true), { ...unc, authority: '', path: '//wsl.localhost/Ubuntu/home/jaket/examples/vstext.mtlx' });
});

test('toSaveDialogDefault leaves other paths, and other platforms, alone', () => {
    const drive = { ...unc, authority: '', path: '/c:/Users/jaket/examples/vstext.mtlx' };
    assert.equal(toSaveDialogDefault(drive, true), drive);
    assert.equal(toSaveDialogDefault(unc, false), unc);
    const remote = { ...unc, scheme: 'vscode-remote', authority: 'wsl+Ubuntu', path: '/home/jaket/examples/vstext.mtlx' };
    assert.equal(toSaveDialogDefault(remote, true), remote);
});
