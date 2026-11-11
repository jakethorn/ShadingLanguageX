// The keywords, types and functions of SLX, from data/language-data.json and
// data/stdlib.mxsl, which gen_extension.py --generate writes.
import * as fs from 'node:fs';
import * as path from 'node:path';
import { blankComments } from './includes';
import { scanSymbols, type ScannedSource, type SlxSymbol } from './symbols';

export interface NodeInput {
    name: string;
    type: string;
    value?: string;
    defaultgeomprop?: string;
    enum?: string;
    uiname?: string;
    uniform?: boolean;
}

export interface NodeOutput {
    name: string;
    type: string;
}

export interface NodeSignature {
    nodedef: string;
    group?: string;
    inputs: NodeInput[];
    outputs: NodeOutput[];
    /** MaterialX versions that define this signature. */
    versions: string[];
}

export interface LanguageDataFile {
    versions: string[];
    keywords: { control: string[]; modifiers: string[]; constants: string[]; other: string[]; all: string[] };
    types: string[];
    aliases: Record<string, string>;
    builtinFunctions: string[];
    nodes: Record<string, NodeSignature[]>;
    /** URL of each node's section in the MaterialX specification, for the nodes it describes. */
    specLinks: Record<string, string>;
}

function compareVersions(a: string, b: string): number {
    const pa = a.split('.').map(Number);
    const pb = b.split('.').map(Number);
    for (let i = 0; i < Math.max(pa.length, pb.length); i++) {
        const diff = (pa[i] ?? 0) - (pb[i] ?? 0);
        if (diff !== 0) return diff;
    }
    return 0;
}

/** Returns the version set by a #version directive, if the source has one. */
export function findVersionDirective(text: string): string | undefined {
    return /^[ \t]*#[ \t]*version[ \t]+([0-9][0-9.]*)/m.exec(blankComments(text))?.[1];
}

export class SlxLanguage {
    readonly keywords: Set<string>;
    readonly types: Set<string>;
    private readonly stdlibFunctions = new Map<string, SlxSymbol[]>();

    constructor(
        readonly data: LanguageDataFile,
        /** stdlib.mxsl, whose functions extend the MaterialX nodes. */
        readonly stdlib: ScannedSource,
        /** Where the copy of stdlib.mxsl is, and its text, so Go to Definition can open it. */
        readonly stdlibFile?: { path: string; text: string },
    ) {
        this.keywords = new Set(data.keywords.all);
        this.types = new Set(data.types);
        for (const symbol of stdlib.symbols) {
            if (symbol.kind !== 'function' || symbol.name.startsWith('__')) continue;
            const overloads = this.stdlibFunctions.get(symbol.name) ?? [];
            overloads.push(symbol);
            this.stdlibFunctions.set(symbol.name, overloads);
        }
    }

    /** Loads the data written by gen_extension.py, or returns undefined if it is missing. */
    static load(dataDir: string): SlxLanguage | undefined {
        try {
            const data = JSON.parse(fs.readFileSync(path.join(dataDir, 'language-data.json'), 'utf8')) as LanguageDataFile;
            const stdlibPath = path.join(dataDir, 'stdlib.mxsl');
            const stdlibText = fs.readFileSync(stdlibPath, 'utf8');
            return new SlxLanguage(data, scanSymbols(stdlibText), { path: stdlibPath, text: stdlibText });
        } catch {
            return undefined;
        }
    }

    get versions(): string[] {
        return this.data.versions;
    }

    /** Returns the closest version there is data for: the version itself, or the newest older one, or the oldest. */
    resolveVersion(version: string): string {
        if (this.versions.includes(version)) return version;
        const older = this.versions.filter((v) => compareVersions(v, version) < 0);
        return older.length > 0 ? older[older.length - 1] : this.versions[0];
    }

    /** Returns the MaterialX name of a type, so that "vec3" is "vector3". */
    canonicalType(type: string): string {
        return this.data.aliases[type] ?? type;
    }

    nodeNames(version: string): string[] {
        const resolved = this.resolveVersion(version);
        return Object.keys(this.data.nodes).filter((name) => this.data.nodes[name].some((s) => s.versions.includes(resolved)));
    }

    nodeSignatures(name: string, version: string): NodeSignature[] {
        const resolved = this.resolveVersion(version);
        return (this.data.nodes[name] ?? []).filter((s) => s.versions.includes(resolved));
    }

    /** Returns the URL of the node's section in the MaterialX specification, if it has one. */
    specLink(name: string): string | undefined {
        return this.data.specLinks?.[name];
    }

    stdlibFunctionNames(): string[] {
        return [...this.stdlibFunctions.keys()];
    }

    stdlibOverloads(name: string): SlxSymbol[] {
        return this.stdlibFunctions.get(name) ?? [];
    }
}
