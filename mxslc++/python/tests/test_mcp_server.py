import asyncio
import os
import sys

import pytest

from mxslc import mcp_server


def test_mcp_tools_registered_with_input_schemas_and_annotations():
    pytest.importorskip("mcp")

    server = mcp_server.create_server()
    tools = {tool.name: tool for tool in asyncio.run(server.list_tools())}

    assert set(tools) == {"compile_mxsl", "decompile_mtlx"}
    assert set(tools["compile_mxsl"].input_schema["properties"]) == {
        "source",
        "version",
        "func_name",
        "sources",
    }
    assert tools["compile_mxsl"].input_schema["required"] == ["source"]
    for tool in tools.values():
        assert tool.annotations.read_only_hint
        assert tool.annotations.idempotent_hint


def test_compiler_errors_preserve_native_messages(monkeypatch):
    pytest.importorskip("mcp")
    from mcp.server.mcpserver.exceptions import ToolError

    def fail(_source, *_args):
        raise RuntimeError("line 1: Invalid expression")

    monkeypatch.setattr(mcp_server, "compile_string_to_string", fail)
    monkeypatch.setattr(mcp_server, "decompile_string_to_string", fail)

    with pytest.raises(ToolError, match="line 1: Invalid expression"):
        mcp_server.compile_mxsl("invalid")
    with pytest.raises(ToolError, match="line 1: Invalid expression"):
        mcp_server.decompile_mtlx("invalid")


def test_server_requires_python_310(monkeypatch):
    monkeypatch.setattr(mcp_server.sys, "version_info", (3, 9))

    with pytest.raises(SystemExit, match="requires Python 3.10 or newer"):
        mcp_server.main()


def test_mcp_stdio_round_trip_is_not_corrupted_by_print():
    pytest.importorskip("mcp")
    from mcp.client.session import ClientSession
    from mcp.client.stdio import StdioServerParameters, stdio_client

    async def run_client():
        params = StdioServerParameters(
            command=sys.executable,
            args=["-m", "mxslc.mcp_server"],
            env={"PYTHONPATH": os.environ["PYTHONPATH"]} if "PYTHONPATH" in os.environ else None,
        )
        async with stdio_client(params) as (read_stream, write_stream):
            async with ClientSession(read_stream, write_stream) as session:
                await session.initialize()
                result = await session.call_tool(
                    "compile_mxsl",
                    {
                        "source": '#include "values.mxsl"\nprint(value);',
                        "sources": {"values.mxsl": "float value = 1.0;"},
                    },
                )
                assert not result.is_error
                assert any("<materialx" in item.text for item in result.content if hasattr(item, "text"))

                invalid = await session.call_tool("compile_mxsl", {"source": "float 1.0"})
                assert invalid.is_error
                assert any(
                    "Invalid statement" in item.text
                    for item in invalid.content
                    if hasattr(item, "text")
                )

    asyncio.run(run_client())
