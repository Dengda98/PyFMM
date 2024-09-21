# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

import sys, os, pathlib
import subprocess 

read_the_docs_build = os.environ.get('READTHEDOCS', None) == 'True'

if read_the_docs_build:
    subprocess.call('cd ..; doxygen doxyfile_h; cd -', shell=True)


html_last_updated_fmt = '%b %d, %Y'
version = "0.1"


def setup(app):
    app.add_css_file('my_theme.css')  


project = 'PyFMM'
copyright = '2023, Zhu Dengda'
author = 'Zhu Dengda'
release = 'v0.1'


# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    "sphinx.ext.napoleon",
    "sphinx.ext.autodoc",
    'sphinx.ext.viewcode',
    "recommonmark",
    "sphinx_markdown_tables",
    "sphinxcontrib.mermaid",
    "sphinx_copybutton",
    "sphinx.ext.intersphinx",
    "breathe",
    'sphinx.ext.mathjax',
    "nbsphinx",
] 

nbsphinx_allow_errors = True  # 在构建文档时允许 Notebook 中的错误
nbsphinx_execute = 'never'   # 不执行 Notebook，只是展示内容


source_suffix = {
    '.rst': 'restructuredtext',
    # '.txt': 'markdown',
    # '.md': 'markdown',
} 

myst_enable_extensions = [
    "tasklist",
    "deflist",
    "dollarmath",
]

# Breathe configuration
breathe_projects = {
    "h_PyFMM": "../doxygen_h/xml",
}
breathe_default_project = "h_PyFMM"

templates_path = ['_templates']
exclude_patterns = []

language = 'zh_CN'

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

# html_theme = 'alabaster'
master_doc = 'index'  
html_theme = 'sphinx_rtd_theme'
html_theme_options = {
    'analytics_anonymize_ip': False,
    'logo_only': False,  # True
    'display_version': True,
    'prev_next_buttons_location': 'bottom',
    'style_external_links': False,
    'collapse_navigation': True,
    'sticky_navigation': True,
    'navigation_depth': 4,
    'includehidden': True,
    'titles_only': False,

}

# html_logo = "./_static/logo.png"
html_static_path = ['_static']
html_js_files = [
    'my_custom.js',
]


autodoc_default_options = {
    'member-order': 'bysource',
    'special-members': '__init__',
}