# Houdini runs this at startup, in the UI and in hython, before any scene file loads.
try:
    import slx_houdini.node_type

    slx_houdini.node_type.install_node_type_at_startup()
except Exception as e:
    print("SLX: could not install the SLX Material node: {}".format(e))
