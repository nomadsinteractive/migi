import os
import sys
from typing import Optional, List, Dict

import json


class Manifest:
    def __init__(self, work_dir: Optional[str] = None, load_libraries: Optional[List[str]] = None, python_home: Optional[str] = None,
                 python_paths: Optional[List[str]] = None, python_scripts: Optional[List[str]] = None, python_console: Optional[List[str]] = None,
                 properties: Optional[Dict[str, str]] = None):
        self._work_dir = work_dir or os.getcwd()
        self._load_libraries = load_libraries
        self._python_home = python_home or self._get_python_home()
        self._python_paths = python_paths
        self._python_scripts = python_scripts
        self._python_console = python_console
        self._properties = properties
        self._manifest = self._load_work_dir_manifest()

    @property
    def python_home(self):
        return self._python_home

    def to_json_str(self) -> str:
        self._update_manifest(self._manifest, work_dir=self._work_dir, load_libraries=self._load_libraries, python_home=self._python_home,
                              python_paths=self._python_paths, python_scripts=self._python_scripts, python_console=self._python_console,
                              properties=self._properties)
        return json.dumps(self._manifest)

    def __str__(self) -> str:
        return self.to_json_str()

    def _load_work_dir_manifest(self):
        manifest_file_path = os.path.join(self._work_dir, 'migi.json')
        if os.path.isfile(manifest_file_path):
            with open(manifest_file_path, 'rt') as fp:
                return json.load(fp)
        return {}

    @staticmethod
    def _get_python_home() -> str:
        dirname, filename = os.path.split(sys.executable)
        return dirname if filename.startswith('python') else None

    @staticmethod
    def _update_manifest(manifest: dict, **kwargs):
        for i, j in kwargs.items():
            if j is not None:
                manifest[i] = j
