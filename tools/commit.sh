git add -A
version=$(cat tools/version)
commitMessage=$(cat commit.md)
formattedCommitMessage="v$version  
$commitMessage"
git commit -m "$formattedCommitMessage"
git tag "v$version"
git push
git push origin "v$version"