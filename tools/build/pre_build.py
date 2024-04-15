import os
import sys
import re

Import("env")

def extract_constant_declarations(cpp_content):
    pattern = r'const\s+char\s*\*\s*(\w+)\s*=\s*"[^"]*";'
    matches = re.finditer(pattern, cpp_content)
    declarations = [f"extern const char *{match.group(1)};" for match in matches]
    return declarations

def create_header_file(cpp_filename, h_filename):
    with open(cpp_filename, 'r', encoding='utf-8') as file:
        cpp_content = file.read()
    
    declarations = extract_constant_declarations(cpp_content)

    if declarations:
        with open(h_filename, 'w', encoding='utf-8') as file:
            file.write('// keys.h\n')
            file.write('#ifndef XZG_KEYS_H\n')
            file.write('#define XZG_KEYS_H\n\n')
            file.writelines("\n".join(declarations))
            file.write('\n\n#endif // XZG_KEYS_H\n')
    else:
        print("No const found !")


KEYS_CPP = "keys.cpp"
KEYS_H = "keys.h"

if os.environ.get("PLATFORMIO_INCLUDE_DIR") is not None:
    KEYS_CPP = (
        os.environ.get("PLATFORMIO_INCLUDE_DIR") + os.sep + KEYS_CPP
    )
    KEYS_H = (
        os.environ.get("PLATFORMIO_INCLUDE_DIR") + os.sep + KEYS_H
    )
elif os.path.exists("src"):
    KEYS_CPP = "src" + os.sep + KEYS_CPP
    KEYS_H = "src" + os.sep + KEYS_H
else:
    PROJECT_DIR = env.subst("$PROJECT_DIR")
    os.mkdir(PROJECT_DIR + os.sep + "include")
    KEYS_CPP = "include" + os.sep + KEYS_CPP
    KEYS_H = "include" + os.sep + KEYS_H


create_header_file(KEYS_CPP, KEYS_H)
