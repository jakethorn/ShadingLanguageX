from mxslc import mcp_server


def test_mcp_tool_wrappers(monkeypatch):
    monkeypatch.setattr(mcp_server, "compile_string_to_string", lambda source: f"compiled: {source}")
    monkeypatch.setattr(mcp_server, "decompile_string_to_string", lambda source: f"decompiled: {source}")

    assert mcp_server.compile_mxsl("float value = 1.0;") == "compiled: float value = 1.0;"
    assert mcp_server.decompile_mtlx("<materialx />") == "decompiled: <materialx />"
