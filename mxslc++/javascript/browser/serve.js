#!/usr/bin/env node
// Minimal static file server for the browser app.
//
// The WASM module fetches JsMxslc.wasm and JsMxslc.data over HTTP, so the page
// cannot be opened via file:// — it must be served. This server serves the
// browser/ folder (where this file lives) with the correct MIME types.
//
// Usage:
//   node serve.js [port]
//   (default port: 8080) -> open http://localhost:8080

import http from 'http';
import { readFile, stat } from 'fs/promises';
import { extname, join, normalize } from 'path';
import { fileURLToPath } from 'url';

const ROOT = fileURLToPath(new URL('./', import.meta.url));
const PORT = parseInt(process.argv[2] || '8080', 10);

// List of MIME types for common file extensions. The server will serve files with
// these MIME types, or application/octet-stream for unknown extensions.
const MIME = {
    '.html': 'text/html; charset=utf-8',
    '.js': 'text/javascript; charset=utf-8',
    '.mjs': 'text/javascript; charset=utf-8',
    '.css': 'text/css; charset=utf-8',
    '.wasm': 'application/wasm',
    '.data': 'application/octet-stream',
    '.json': 'application/json; charset=utf-8',
    '.png': 'image/png',
    '.svg': 'image/svg+xml',
    '.ico': 'image/x-icon',
    '.txt': 'text/plain; charset=utf-8',
    '.mtlx': 'application/xml; charset=utf-8',
    '.mxsl': 'text/plain; charset=utf-8',
};

// Create the HTTP server. It serves files from ROOT.
const server = http.createServer(async (req, res) => {
    try {
        // Resolve the path safely within ROOT. Derive the base URL from the
        // request's own Host header (falling back to localhost only if absent)
        // rather than assuming localhost, so the server works when reached via
        // any host/port, and handles absolute req.url values (e.g. behind a proxy).
        const base = `http://${req.headers.host || 'localhost'}`;
        let pathname = decodeURIComponent(new URL(req.url, base).pathname);
        if (pathname === '/') pathname = '/index.html';
        const filePath = normalize(join(ROOT, pathname));

        if (!filePath.startsWith(ROOT)) {
            res.writeHead(403).end('Forbidden');
            return;
        }

        const st = await stat(filePath);
        if (!st.isFile()) {
            res.writeHead(404).end('Not found');
            return;
        }

        const type = MIME[extname(filePath).toLowerCase()] || 'application/octet-stream';
        const data = await readFile(filePath);
        res.writeHead(200, {
            'Content-Type': type,
            'Content-Length': data.length,
            'Cache-Control': 'no-cache',
        });
        res.end(data);
    } catch (err) {
        if (err && err.code === 'ENOENT') {
            res.writeHead(404).end('Not found');
        } else {
            res.writeHead(500).end('Internal error');
        }
    }
});

// Start the server and log the address. If the server is bound to a wildcard address,
// also log that it is reachable via localhost and any host/IP.
server.listen(PORT, () => {
    const addr = server.address();
    const bound = (addr && addr.address) || 'localhost';
    const isWildcard = bound === '0.0.0.0' || bound === '::' || bound === '::0';
    const displayHost = bound.includes(':') ? `[${bound}]` : bound;
    console.log(`Serving ${ROOT}`);
    console.log(`  -> http://${displayHost}:${PORT}`);
    if (isWildcard) {
        console.log(`  (bound to ${bound}; also reachable via http://localhost:${PORT} and any host/IP)`);
    }
});
