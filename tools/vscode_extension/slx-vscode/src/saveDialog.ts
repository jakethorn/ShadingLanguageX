// Where a save dialog starts.
export interface UriParts {
    scheme: string;
    authority: string;
    path: string;
    query: string;
    fragment: string;
}

/**
 * Returns the default file to give vscode.window.showSaveDialog, working around VS Code's handling of Windows UNC
 * paths such as \\wsl.localhost\Ubuntu\home\... .
 *
 * VS Code passes the native dialog only the path of the default file, as URI.file(defaultUri.path). That loses the
 * server (wsl.localhost), so the dialog opens the folder it used last instead. Moving the server into the path as
 * //wsl.localhost/Ubuntu/home/... survives this: URI.file reads it back as the same UNC path, and so would a VS Code
 * that used the file's own fsPath instead.
 */
export function toSaveDialogDefault(uri: UriParts, isLocalWindows: boolean): UriParts {
    if (!isLocalWindows || uri.scheme !== 'file' || !uri.authority) return uri;
    return { scheme: 'file', authority: '', path: `//${uri.authority}${uri.path}`, query: uri.query, fragment: uri.fragment };
}
