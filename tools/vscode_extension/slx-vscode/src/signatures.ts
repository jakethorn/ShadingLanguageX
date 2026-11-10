// Formats the signatures of MaterialX nodes and SLX functions for hovers,
// parameter hints and completion.
import type { NodeInput, NodeSignature } from './languageData';
import type { SlxSymbol } from './symbols';

export interface ParameterHint {
    name: string;
    /** Start and end of the parameter in the signature's label. */
    range: [number, number];
    documentation?: string;
}

export interface SignatureHint {
    label: string;
    documentation?: string;
    parameters: ParameterHint[];
}

const VALUE_TYPES = new Set(['float', 'integer', 'boolean']);

/** Returns an input's default value as SLX, e.g. `color3{0.8, 0.8, 0.8}`, or undefined if it has none. */
export function formatDefaultValue(input: NodeInput): string | undefined {
    if (input.value === undefined || input.defaultgeomprop !== undefined) return undefined;
    if (input.type === 'string' || input.type === 'filename') return JSON.stringify(input.value);
    if (VALUE_TYPES.has(input.type) || input.value === '') return input.value;
    return `${input.type}{${input.value}}`;
}

/** Describes an input in a sentence or two: what it is, its default and the values it accepts. */
export function describeInput(input: NodeInput): string {
    const parts: string[] = [];
    if (input.uiname) parts.push(/[.!?]$/.test(input.uiname) ? input.uiname : `${input.uiname}.`);
    const defaultValue = formatDefaultValue(input);
    if (input.defaultgeomprop) parts.push(`Defaults to the \`${input.defaultgeomprop}\` geometric property.`);
    else if (defaultValue !== undefined && defaultValue !== '""') parts.push(`Defaults to \`${defaultValue}\`.`);
    if (input.enum) parts.push(`One of: ${input.enum.split(',').map((v) => `\`${v.trim()}\``).join(', ')}.`);
    if (input.uniform) parts.push('Uniform.');
    return parts.join(' ');
}

function formatInput(input: NodeInput): string {
    return `${input.uniform ? 'uniform ' : ''}${input.type} ${input.name}`;
}

function formatReturnType(signature: NodeSignature): string {
    if (signature.outputs.length === 1) return signature.outputs[0].type;
    return `{${signature.outputs.map((o) => `${o.type} ${o.name}`).join(', ')}}`;
}

/**
 * True when the signature can only be told apart from another one by its
 * output type, so a call has to give it, as in `image<color3>(...)`.
 */
function needsTemplateArgument(signature: NodeSignature, all: NodeSignature[]): boolean {
    const inputs = signature.inputs.map((i) => i.type).join(',');
    return all.some((other) => other !== signature && other.inputs.map((i) => i.type).join(',') === inputs);
}

function nodeSignatureLabel(name: string, signature: NodeSignature, all: NodeSignature[]): { label: string; ranges: Array<[number, number]> } {
    const template = needsTemplateArgument(signature, all) && signature.outputs.length === 1 ? `<${signature.outputs[0].type}>` : '';
    let label = `${formatReturnType(signature)} ${name}${template}(`;
    const ranges: Array<[number, number]> = [];
    signature.inputs.forEach((input, index) => {
        if (index > 0) label += ', ';
        const text = formatInput(input);
        ranges.push([label.length, label.length + text.length]);
        label += text;
    });
    return { label: `${label})`, ranges };
}

/** Returns a Markdown link to the node's section in the MaterialX specification. */
export function formatSpecLink(specLink: string): string {
    return `[MaterialX specification](${specLink})`;
}

export function nodeSignatureHints(name: string, signatures: NodeSignature[], specLink?: string): SignatureHint[] {
    return signatures.map((signature) => {
        const { label, ranges } = nodeSignatureLabel(name, signature, signatures);
        return {
            label,
            documentation: specLink ? formatSpecLink(specLink) : undefined,
            parameters: signature.inputs.map((input, index) => ({
                name: input.name,
                range: ranges[index],
                documentation: describeInput(input) || undefined,
            })),
        };
    });
}

export function functionSignatureHint(symbol: SlxSymbol): SignatureHint {
    const label = symbol.signature;
    const open = label.indexOf('(');
    const parameters: ParameterHint[] = [];
    let searchFrom = open + 1;
    for (const parameter of symbol.parameters ?? []) {
        const start = label.indexOf(parameter.label, searchFrom);
        if (start === -1) continue;
        parameters.push({ name: parameter.name, range: [start, start + parameter.label.length], documentation: parameter.doc });
        searchFrom = start + parameter.label.length;
    }
    return { label, documentation: symbol.doc, parameters };
}

/** How many signatures a hover lists before summarising the rest. */
const HOVER_SIGNATURE_LIMIT = 8;

/** Returns the hover text, as Markdown, for a MaterialX node, linking to its section of the specification if it has one. */
export function formatNodeHover(name: string, signatures: NodeSignature[], version: string, specLink?: string): string {
    const lines: string[] = [];
    if (signatures.length === 1) {
        // One signature: one input per line, with defaults, as most such nodes have many inputs.
        const signature = signatures[0];
        const template = nodeSignatureLabel(name, signature, signatures).label.split('(')[0];
        lines.push('```slx', `${template}(`);
        signature.inputs.forEach((input, index) => {
            const defaultValue = formatDefaultValue(input);
            const comma = index < signature.inputs.length - 1 ? ',' : '';
            const comment = input.defaultgeomprop ? ` // ${input.defaultgeomprop}` : input.enum ? ` // ${input.enum.split(',').join(', ')}` : '';
            lines.push(`    ${formatInput(input)}${defaultValue !== undefined ? ` = ${defaultValue}` : ''}${comma}${comment}`);
        });
        lines.push(')', '```');
    } else {
        lines.push('```slx');
        for (const signature of signatures.slice(0, HOVER_SIGNATURE_LIMIT)) lines.push(nodeSignatureLabel(name, signature, signatures).label);
        lines.push('```');
        if (signatures.length > HOVER_SIGNATURE_LIMIT) lines.push(`*…and ${signatures.length - HOVER_SIGNATURE_LIMIT} more signatures*`, '');
    }
    const group = signatures.find((s) => s.group)?.group;
    lines.push(`MaterialX ${version} node${group ? ` · ${group}` : ''}${specLink ? ` · ${formatSpecLink(specLink)}` : ''}`);
    return lines.join('\n');
}

/** Returns the hover text, as Markdown, for functions and other declarations written in SLX. */
export function formatDeclarationHover(symbols: SlxSymbol[], origin?: string): string {
    const lines = ['```slx', ...symbols.slice(0, HOVER_SIGNATURE_LIMIT).map((s) => s.signature), '```'];
    if (symbols.length > HOVER_SIGNATURE_LIMIT) lines.push(`*…and ${symbols.length - HOVER_SIGNATURE_LIMIT} more*`, '');
    if (origin) lines.push(origin);
    const doc = symbols.find((s) => s.doc)?.doc;
    if (doc) lines.push('', doc);
    return lines.join('\n');
}

/** Returns the hover text, as Markdown, for a named argument of a node, e.g. `base_color` in `standard_surface(base_color = c)`. */
export function formatInputHover(nodeName: string, input: NodeInput): string {
    return ['```slx', `${formatInput(input)} // ${nodeName} input`, '```', describeInput(input)].join('\n');
}
