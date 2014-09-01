#!/bin/bash
CURRENT_BRANCH=`git branch |grep "*" | sed 's/* //g'`
echo "Current branch is: $CURRENT_BRANCH"
#git fetch upstream-ro-fm
git checkout trunk && git fetch upstream-ro && git pull upstream-ro +trunk:trunk
#git fetch upstream-ro-xbmc
#git checkout master-xbmc && git fetch upstream-ro-xbmc && git pull upstream-ro-xbmc +master:master-xbmc
#attualizza 
git checkout $CURRENT_BRANCH
