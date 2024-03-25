#!/bin/sh

# Add git.hook to remove .version_no_increment file before commit

cd ..
echo "python ./tools/version_increment_post.py" > ./.git/hooks/pre-commit
chmod +x ./.git/hooks/pre-commit
