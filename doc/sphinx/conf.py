# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# http://www.sphinx-doc.org/en/master/config

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
# import os
# import sys
# sys.path.insert(0, os.path.abspath('.'))


# -- Project information -----------------------------------------------------

project = 'Seraphim'
copyright = '2019, Christopher N. Hesse'
author = 'Christopher N. Hesse'


# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = ['breathe', 'exhale']

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']

# Tell sphinx what the primary language being documented is.
primary_domain = 'cpp'

# Tell sphinx what the pygments highlight language should be.
highlight_language = 'cpp'


# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'sphinx_rtd_theme'

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named 'default.css' will overwrite the builtin 'default.css'.
html_static_path = ['_static']


# -- Breathe configuration ---------------------------------------------------
breathe_projects = { 'Seraphim': '../../build/doc/doxygen/xml' }
breathe_default_project = 'Seraphim'

# -- Exhale configuration ----------------------------------------------------
exhale_args = {
    # These arguments are required
    'containmentFolder':     './api',
    'rootFileName':          'root.rst',
    'rootFileTitle':         'Seraphim API',
    'doxygenStripFromPath':  '..',
    # Suggested optional arguments
    'createTreeView':        True,
    # TIP: if using the sphinx-bootstrap-theme, you need
    # 'treeViewIsBootstrap': True,
    # We achieve the same by making the exhale makefile target depend on the
    # doxy one
    #'exhaleExecutesDoxygen': True,
    #'exhaleDoxygenStdin':    'INPUT = ../../src/lib'
}
