Python API
==========

The ``mxslc`` module, installed with ``pip install mxslcxx``. This reference is generated from the module's type stubs,
which the wheel also includes, so editors show the same signatures.

.. toctree::
   :titlesonly:

   {% for page in pages|selectattr("is_top_level_object") %}
   {{ page.include_path }}
   {% endfor %}
