echo off
:build
node gulp.js
Title "UZG-01 webfiles builder"
echo Press any key for build again or close window
pause
goto build
