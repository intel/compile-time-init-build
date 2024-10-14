import pytest
import hypothesis 
import json
import subprocess
import tempfile
import os
import re

hypothesis.settings.register_profile("ci", max_examples=500)
hypothesis.settings.register_profile("fast", max_examples=10)

    

def pytest_addoption(parser):
    parser.addoption("--compiler", action="store", help="C++ compiler", default=None, required=False)
    parser.addoption("--compiler-args", action="store", help="C++ compiler arguments", default="", required=False)
    parser.addoption("--includes", action="store", help="C++ include directories", default="", required=False)

    parser.addoption("--compile-commands", action="store", help="cmake compiler commands", default=None, required=False)
    parser.addoption("--prototype-driver", action="store", help="Prototype .cpp filename to gather compilation command from", default=None, required=False)
    
@pytest.fixture(scope="module")
def cmake_compilation_command(pytestconfig):
    compile_commands_filename = pytestconfig.getoption("compile_commands")
    prototype_driver_filename = pytestconfig.getoption("prototype_driver")

    if compile_commands_filename is None or prototype_driver_filename is None:
        return None

    def f(filename):
        with open(compile_commands_filename, "r") as f:
            db = json.load(f)
            for obj in db:
                if obj["file"] == prototype_driver_filename:
                    cmd = obj["command"]
                    cmd = cmd.replace(prototype_driver_filename, filename)
                    cmd = re.sub(r"-o .*?\.cpp\.o", f"-o {filename}.o", cmd)
                    cmd = re.sub(r" -c ", f" ", cmd)
                    return cmd.split(" ")

    return f

@pytest.fixture(scope="module")
def args_compilation_command(pytestconfig):
    compiler = pytestconfig.getoption("compiler")
    if compiler is None:
        return None

    include_dirs = [f"-I{i}" for i in pytestconfig.getoption("includes").split(",") if i]
    compiler_args = [i for i in pytestconfig.getoption("compiler_args").split(",") if i]   
    
    def f(filename):
        compile_command = [
            compiler, temp_cpp_file_path, 
            "-o", temp_cpp_file_path + ".out"
        ] + compiler_args + include_args
        return compile_command
    
    return f



@pytest.fixture(scope="module")
def compile(cmake_compilation_command, args_compilation_command):
    cmd = cmake_compilation_command
    if cmd is None:
        cmd = args_compilation_command
        
    def f(code_str):
        code_str += "\n"
        with tempfile.NamedTemporaryFile(delete=False, suffix=".cpp") as temp_cpp_file:
            temp_cpp_file.write(code_str.encode('utf-8'))
            temp_cpp_file_path = temp_cpp_file.name
        
        try:
            compile_command = cmd(temp_cpp_file_path)
            result = subprocess.run(compile_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            
            if result.returncode == 0:
                os.chmod(temp_cpp_file_path + ".o", 0o700)
                return temp_cpp_file_path + ".o"
            else:
                error_message = (
                    f"Compiler returned non-zero exit code: {result.returncode}\n"
                    f"Compilation command: {' '.join(compile_command)}\n"
                    f"Source code:\n{code_str}\n"
                    f"Compiler stderr:\n{result.stderr.decode('utf-8')}\n"
                    f"Compiler stdout:\n{result.stdout.decode('utf-8')}\n"
                )
                pytest.fail(error_message)
        
        except Exception as e:
            pytest.fail(str(e))
        finally:
            os.remove(temp_cpp_file_path)

    return f

