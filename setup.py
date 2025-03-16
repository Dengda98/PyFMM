from setuptools import setup, find_packages
import os



# 读取版本号
def read_version():
    version_file = os.path.join('pyfmm', '_version.py')
    with open(version_file) as f:
        exec(f.read())
    return locals()['__version__']

def read_readme():
    with open("README.md", encoding="utf-8") as f:
        return f.read()

setup(
    name='pyfmm-kit',
    version=read_version(),
    description='A C/Python package for solving eikonal equation using Fast Marching/Sweeping Method',
    author='Zhu Dengda',
    author_email='zhudengda@mail.iggcas.ac.cn',
    long_description=read_readme(),  
    long_description_content_type="text/markdown", 
    url="https://github.com/Dengda98/PyFMM",
    project_urls={
        "Documentation": "https://pyfmm.readthedocs.io/zh-cn/latest/",
        "Source Code": "https://github.com/Dengda98/PyFMM",
    },

    packages=find_packages(),
    package_data={'pyfmm': ['./C_extension/*']},
    include_package_data=True,
    install_requires=[
        'numpy>=1.20, <2.0',
        'scipy>=1.10',
        'matplotlib>=3.5',
        'jupyter',
    ],
    python_requires='>=3.6',
    zip_safe=False,  # not compress the binary file
)
