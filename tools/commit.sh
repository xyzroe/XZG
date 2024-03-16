git add -A 
git commit -F ../commit
version=$(cat version)
git tag "v$version"
git push
git push origin "v$version"