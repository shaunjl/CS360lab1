all: download
download: download.cpp
	g++ -o download download.cpp
git_local:
	git add .
	git commit -m "dev"
	git push origin master
git_remote:
	git stash save
	git pull
	git stash drop
	g++ -o download download.cpp
