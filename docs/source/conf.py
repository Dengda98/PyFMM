# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

import os

read_the_docs_build = os.environ.get('READTHEDOCS', None) == 'True'

html_last_updated_fmt = '%b %d, %Y'

def setup(app):
    app.add_css_file('my_theme.css')  


# 读取版本号
def read_version():
    version_file = os.path.join('../../pyfmm', '_version.py')
    with open(version_file) as f:
        exec(f.read())
    return locals()['__version__']

project = 'PyFMM'
copyright = '2025, Zhu Dengda'
author = 'Zhu Dengda'
# 引入版本信息
version = read_version()
release = version

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

nbsphinx_allow_errors = False  # Notebook 执行出错时终止文档构建
nbsphinx_execute = 'always' if read_the_docs_build else 'never'  # RTD 每次重新执行，本地默认展示已保存的结果
nbsphinx_kernel_name = 'python3'
nbsphinx_execute_arguments = ['--matplotlib=inline']  # 在页面中保留本次执行生成的图像
nbsphinx_timeout = 600  # 每个单元格最多执行 600 秒


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