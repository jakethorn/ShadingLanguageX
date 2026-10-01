import asyncio

from . import compile_string_to_string, decompile_string_to_string


def compile_mxsl(source: str) -> str:
    """Compile ShadingLanguageX source into MaterialX XML."""
    return compile_string_to_string(source)


def decompile_mtlx(source: str) -> str:
    """Decompile MaterialX XML into ShadingLanguageX source."""
    return decompile_string_to_string(source)


def create_server():
    from mcp.server.mcpserver import MCPServer

    server = MCPServer(name="ShadingLanguageX")
    server.tool(description="Compile ShadingLanguageX source into MaterialX XML.")(compile_mxsl)
    server.tool(description="Decompile MaterialX XML into ShadingLanguageX source.")(decompile_mtlx)
    return server


def main() -> None:
    try:
        server = create_server()
    except ImportError as exc:
        raise SystemExit('Install the MCP extra with `pip install "mxslcxx[mcp]"` to run this server.') from exc

    asyncio.run(server.run_stdio_async())
