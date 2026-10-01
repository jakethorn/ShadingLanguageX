from __future__ import annotations

import sys

from . import CompileOptions, compile_string_to_string, decompile_string_to_string


def compile_mxsl(
    source: str,
    version: str = "1.39.5",
    func_name: str | None = None,
    sources: dict[str, str] | None = None,
) -> str:
    """Compile ShadingLanguageX source into MaterialX XML.

    ``sources`` maps virtual include paths to their source contents.
    """
    try:
        options = CompileOptions(version=version, func_name=func_name, sources=sources or {})
        return compile_string_to_string(source, options)
    except RuntimeError as exc:
        from mcp.server.mcpserver.exceptions import ToolError

        raise ToolError(str(exc)) from exc


def decompile_mtlx(source: str) -> str:
    """Decompile MaterialX XML into ShadingLanguageX source."""
    try:
        return decompile_string_to_string(source)
    except RuntimeError as exc:
        from mcp.server.mcpserver.exceptions import ToolError

        raise ToolError(str(exc)) from exc


def create_server():
    from mcp.server.mcpserver import MCPServer
    from mcp.types import ToolAnnotations

    server = MCPServer(name="ShadingLanguageX")
    annotations = ToolAnnotations(readOnlyHint=True, idempotentHint=True)
    server.tool(annotations=annotations)(compile_mxsl)
    server.tool(annotations=annotations)(decompile_mtlx)
    return server


def main() -> None:
    if sys.version_info < (3, 10):
        raise SystemExit("The ShadingLanguageX MCP server requires Python 3.10 or newer.")

    try:
        server = create_server()
    except ImportError as exc:
        raise SystemExit('Install the MCP extra with `pip install "mxslcxx[mcp]"` to run this server.') from exc

    server.run("stdio")


if __name__ == "__main__":
    main()
