!#/bin/zsh

set -e # close on errors

git add .
git commit -m "${1}"
git push main
