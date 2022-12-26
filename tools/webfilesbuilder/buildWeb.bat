echo off
:build
node gulp.js
Title "SLZB-06 webfiles builder"
echo Press any key for build again or close window
pause
goto build
