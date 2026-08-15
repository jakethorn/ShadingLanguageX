# browser_shared

Single source of truth for the files shared by both browser integrations:

| File          | Purpose                                                    |
|---------------|------------------------------------------------------------|
| `index.html`  | The page markup + CSS (both integrations)                  |
| `common.js`   | Engine-agnostic UI core (editors, log, status, conversion) |
| `keywords.js` | Static MXSL keyword backup for syntax highlighting         |

`app.js` is **not** shared — it is the per-integration engine adapter
(Flask `fetch` vs WASM) and lives in each integration's folder.

## How it works

Each integration keeps its own physical copy so its served folder stays
self-contained for deployment:

```
browser_shared/            <-- edit these (canonical)
    index.html
    common.js
    keywords.js

python/browser/            (Flask)
    templates/index.html
    static/app.js          (Flask engine)
    static/common.js
    static/keywords.js

javascript/browser/        (WASM)
    index.html
    app.js                 (WASM engine)
    common.js
    keywords.js
```

The only difference between the two `index.html` copies is the final `<script>`
`src` path (`static/app.js` for Flask vs `app.js` for WASM), because each app
serves static files from a different root.

## Commands

From `mxslc++/`:

```sh
./sync_browser.sh            # copy shared files into both integrations
./sync_browser.sh --check    # verify copies are in sync (exit 1 on drift)
```

Or via npm (from `mxslc++/javascript/`):

```sh
npm run sync                 # copy
npm run sync:check           # verify
npm run build                # also syncs first (prebuild)
```

## Automation

- `npm run build` runs the sync automatically via `prebuild`.
- A pre-commit hook runs `--check` to block commits that drift:
  `git config core.hooksPath .githooks` (from the repo root).
- Add `mxslc++/sync_browser.sh --check` to CI to catch drift on every run.

## Workflow

Edit files under `browser_shared/`, then run `./sync_browser.sh` (or build) to
propagate. Never edit the generated copies in the integration folders directly —
they will be overwritten and the drift check will flag them.
