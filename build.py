from pathlib import Path
import shutil
import subprocess
import re

class FileManager:
    def mkdir(self, path):
        Path(path).mkdir(parents=True, exist_ok=True) 
    
    def read(self, path):
        return Path(path).read_text()

    def remove(self, path):
        Path(path).unlink(missing_ok=True)
    
    def write(self, path, content):
        Path(path).parent.mkdir(parents=True, exist_ok=True)
        Path(path).write_text(content)

    def copy(self, src, dst):
        shutil.copy(src, dst)

class TempFileSystem:
    def __init__(self, basedir=".cache"):
        self.basedir = Path(basedir)
        self.basedir.mkdir(parents=True, exist_ok=True)
        self.file_manager = FileManager()
        self.files = []

    def create_file(self, filename, content):
        path = self.basedir / filename
        self.file_manager.write(path, content)
        self.files.append(path)
        return path
    
    def register_file(self, filename):
        path = self.basedir / filename
        self.files.append(path)
        return path
    
    def copy_file(self, src):
        path = self.basedir / Path(src).name
        self.file_manager.copy(src, path)
        self.files.append(path)
    
    def remove_all(self):
       for file in self.files:
            self.file_manager.remove(file) 

class RiscVCompiler:
    def __init__(self):
        self.gcc = shutil.which("riscv64-elf-gcc")
        if not self.gcc:
            raise Exception("Cannot find riscv64-elf-gcc")

    def compile(self, input_file, output_file, include_dirs=None):
        cmd = [
            str(self.gcc),
            "-S",
            "-march=rv32i",
            "-mabi=ilp32",
            "-fno-builtin",
            "-nostdlib",
        ]
        if include_dirs:
            for d in include_dirs:
                cmd.extend(["-I", str(d)])
        cmd.extend([str(input_file), "-o", str(output_file)])
        result = subprocess.run(cmd)
        if result.returncode != 0:
            raise RuntimeError(f"Compilation error: {result.stderr}")

class RarsAdapter:
    SKIP = [
        re.compile(r'^\s*\.file\b'),
        re.compile(r'^\s*\.option\b'),
        re.compile(r'^\s*\.attribute\b'),
        re.compile(r'^\s*\.type\b'),
        re.compile(r'^\s*\.size\b'),
        re.compile(r'^\s*\.ident\b'),
        re.compile(r'^\s*\.section\s+\.note\.GNU-stack'),
    ]

    SECTION_REPLACE = [
        (re.compile(r'^\s*\.section\s+\.rodata\b.*'), '\t.data'),
        (re.compile(r'^\s*\.section\s+\.sbss\b.*'),   '\t.data'),
    ]

    LOCAL_LABEL = re.compile(r'\.L([A-Za-z0-9_]+)')
    ZERO = re.compile(r'^(\s*)\.zero\s+(\d+)')

    def process(self, content, filename):
        result = []
        for line in content.splitlines():
            skip = False
            for pat in self.SKIP:
                if pat.match(line):
                    skip = True
                    break
            if skip:
                continue

            for pat, repl in self.SECTION_REPLACE:
                if pat.match(line):
                    line = repl
                    break

            m = self.ZERO.match(line)
            if m:
                line = f'{m.group(1)}.space {m.group(2)}'

            line = self.LOCAL_LABEL.sub(rf'{filename}_L\1', line)

            result.append(line)
        return '\n'.join(result)

    def combine(self, parts):
        header = (
            ".text\n"
            ".globl _start\n"
            "_start:\n"
            "\tcall main\n"
            "\tli a7, 10\n"
            "\tecall\n"
        )
        return header + '\n' + '\n\n'.join(parts) + '\n'


MAIN_SOURCES = [
    Path("src/main.c"),
    Path("src/tokenizer.c"),
    Path("src/parser.c"),
    Path("src/lib/io.S"),
    Path("src/lib/memory.S"),
]

LIB_SOURCES = [
    Path("src/lib/string.c"),
    Path("src/lib/test.c"),
    Path("src/lib/io.S"),
    Path("src/lib/memory.S"),
]

TEST_CONFIGS = {
    "tokenizer": {
        "sources": [
            Path("tests/test_tokenizer.c"),
            Path("src/tokenizer.c"),
        ] + LIB_SOURCES,
        "output": "test_tokenizer.s",
    },
}

def build(sources, output):
    fm = FileManager()
    tfs = TempFileSystem()
    compiler = RiscVCompiler()
    adapter = RarsAdapter()
    parts = []

    for source in sources:
        if source.suffix == '.c':
            asm_name = source.stem + ".s"
            asm_path = tfs.register_file(asm_name)
            compiler.compile(source, asm_path, include_dirs=["src"])
            raw = fm.read(asm_path)
            parts.append(adapter.process(raw, source.stem))
        elif source.suffix == '.S':
            tfs.copy_file(source)
            raw = fm.read(tfs.basedir / source.name)
            parts.append(raw)

    program = adapter.combine(parts)
    fm.write(output, program)
    print(f"{output} ({program.count(chr(10))} lines)")

if __name__ == "__main__":
    build(MAIN_SOURCES, "program.s")
    for test_name, cfg in TEST_CONFIGS.items():
        print(f"--- test: {test_name} ---")
        build(cfg["sources"], cfg["output"])
