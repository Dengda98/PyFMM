from setuptools import setup, find_packages
from setuptools.command.build import build as build_orig
from setuptools.command.develop import develop as develop_orig
from setuptools.command.install import install as install_orig
import subprocess
import os, sys


class BuildMake(build_orig):
    def run(self):
        # 执行 make 命令
        make_cmd = 'cd pyfmm/C_extension && make clean && make'
        process = subprocess.Popen(make_cmd, stdout=sys.stdout, stderr=sys.stderr, shell=True)
        process.wait()
        if process.returncode != 0:
            raise subprocess.CalledProcessError(process.returncode, 'make')
        
        super().run()

# 强制install执行build
class Install(install_orig):
    def run(self):
        self.run_command('build')
        super().run()

# 强制develop执行build
class Develop(develop_orig):
    def run(self):
        self.run_command('build')
        super().run()


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
    cmdclass={
        'build': BuildMake,
        'install': Install,
        'develop': Develop,  # 添加 Develop 类到 cmdclass 中
    },
    install_requires=[
        'numpy>=1.20, <2.0',
        'scipy>=1.10',
        'matplotlib>=3.5',
        'jupyter',
    ],
    python_requires='>=3.9',
)
