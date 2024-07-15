import os
Import("env")

# include toolchain paths
env.Replace(COMPILATIONDB_INCLUDE_TOOLCHAIN=True)

# override compilation DB path
# use this to write compile commands into build dir, but
# I personally just want it in the base dir
#env.Replace(COMPILATIONDB_PATH=os.path.join("$SRC_DIR", "compile_commands.json"))
