// Reads the node definitions in a MaterialX file that SLX files load with #library, so their nodes can be completed
// and described like the standard ones.
import type { NodeInput, NodeOutput, NodeSignature } from './languageData';

export interface LibraryNode {
    /** The node's name, which SLX calls it by. */
    name: string;
    signature: NodeSignature;
    /** Offset of the <nodedef> element in the file's text. */
    offset: number;
}

const ENTITIES: Record<string, string> = { quot: '"', apos: "'", lt: '<', gt: '>', amp: '&' };

function readAttributes(tag: string): Record<string, string> {
    const attributes: Record<string, string> = {};
    for (const [, name, value] of tag.matchAll(/([\w:-]+)\s*=\s*"([^"]*)"/g)) {
        attributes[name] = value.replace(/&(quot|apos|lt|gt|amp);/g, (_, entity: string) => ENTITIES[entity]);
    }
    return attributes;
}

/** Returns the nodes defined by the <nodedef> elements in a MaterialX document. */
export function parseNodeDefs(text: string): LibraryNode[] {
    // Blank out comments, keeping offsets.
    const xml = text.replace(/<!--[\s\S]*?-->/g, (comment) => ' '.repeat(comment.length));
    const nodes: LibraryNode[] = [];
    for (const match of xml.matchAll(/<nodedef\b([^>]*?)(?:\/>|>([\s\S]*?)<\/nodedef\s*>)/g)) {
        const nodedef = readAttributes(match[1]);
        if (!nodedef.node) continue;
        const inputs: NodeInput[] = [];
        const outputs: NodeOutput[] = [];
        for (const [, tag, attributeText] of (match[2] ?? '').matchAll(/<(input|output)\b([^>]*?)\/?>/g)) {
            const port = readAttributes(attributeText);
            if (!port.name || !port.type) continue;
            if (tag === 'output') {
                outputs.push({ name: port.name, type: port.type });
                continue;
            }
            const input: NodeInput = { name: port.name, type: port.type };
            if (port.value !== undefined) input.value = port.value;
            if (port.defaultgeomprop) input.defaultgeomprop = port.defaultgeomprop;
            if (port.enum) input.enum = port.enum;
            if (port.uiname) input.uiname = port.uiname;
            if (port.uniform === 'true') input.uniform = true;
            inputs.push(input);
        }
        // Before MaterialX 1.38, a node definition with one output could give its type instead.
        if (outputs.length === 0 && nodedef.type) outputs.push({ name: 'out', type: nodedef.type });
        nodes.push({
            name: nodedef.node,
            signature: { nodedef: nodedef.name ?? '', group: nodedef.nodegroup, inputs, outputs, versions: [] },
            offset: match.index,
        });
    }
    return nodes;
}
