echo off
:build
node gulp.js
Title "Webfiles builder"
echo Press any key for build again or close window
pause
goto build
