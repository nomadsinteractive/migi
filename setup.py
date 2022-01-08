import os
import shutil
import subprocess
from pathlib import Path
from typing import Optional

import setuptools
from setuptools.command.build_ext import build_ext
from setuptools.extension import Extension


class CMakeInstallCmdClass(build_ext):
    description = "Installs compoents from the CMake binary directory."
    user_options = [
        ('cmake-binary-dir=', None, 'Specify the CMake binary directory to install the components.'),
    ]

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def initialize_options(self):
        super().initialize_options()
        self.cmake_binary_dir = None

    def finalize_options(self):
        super().finalize_options()
        self.cmake_binary_dir = self.cmake_binary_dir or os.environ['CMAKE_BINARY_DIR']

    def run(self):
        if shutil.which("cmake") is None:
            raise RuntimeError("Required command 'cmake' not found")

        cmake_extensions = [e for e in self.extensions if isinstance(e, CMakeInstall)]

        for i in cmake_extensions:
            ext_fullpath = self.get_ext_fullpath(i.name)
            for j in self.cmake_binary_dir.split(os.pathsep):
                self._check_cmake_binary_dir(j)
                i.install(ext_fullpath, j)

    def get_source_files(self):
        return []

    @staticmethod
    def _check_cmake_binary_dir(i: str):
        assert os.path.isdir(i) and os.path.isfile(os.path.join(i, 'CMakeCache.txt')),\
               'cmake-binary-dir("%s") doesn\'t seem to be a CMake build directory' % i


class CMakeInstall(Extension):
    def __init__(self, name: str, install_prefix: Optional[str] = None, component_name: Optional[str] = None):
        super().__init__(name, [])
        self._install_prefix = install_prefix
        self._component_name = component_name

    def install(self, ext_fullpath: str, cmake_binary_dir: str):
        ext_dir = Path(ext_fullpath).parent.absolute()
        cmake_install_prefix = ext_dir / self._install_prefix if self._install_prefix else ext_dir

        install_command = ["cmake", "--install", cmake_binary_dir, '--prefix', str(cmake_install_prefix)]

        if self._component_name:
            install_command.extend(["--component", self._component_name])

        subprocess.check_call(install_command)


setuptools.setup(
    ext_modules=[
        CMakeInstall(name='migi_bindings_limited_api', install_prefix='migi', component_name='migi_bindings_limited_api'),
        CMakeInstall(name='migi_injectors', install_prefix='migi', component_name='migi_injector'),
    ],
    cmdclass={'build_ext': CMakeInstallCmdClass}
)
