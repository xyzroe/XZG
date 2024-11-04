var gulp = require('gulp');
var concat = require('gulp-concat');
var gzip = require('gulp-gzip');
var htmlmin = require('gulp-htmlmin');
var uglify = require('gulp-uglify');
var cssnano = require('gulp-cssnano');
const jsonminify = require('gulp-jsonminify');

function stylesConcat() {
    return gulp.src(['../../src/websrc/css/custom.css', '../../src/websrc/css/bootstrap.min.css'])
        .pipe(concat({
            path: 'style.css',
            stat: {
                mode: 0666
            }
        }))
        .pipe(cssnano({preset: 'advanced'}))
        .pipe(gulp.dest('../../src/websrc/min/css/'))
        .pipe(gzip({
            append: true
        }))
        .pipe(gulp.dest('../../data/css/'));
}

function scriptsgz() {
    return gulp.src("../../src/websrc/js/*.js")
        .pipe(uglify())
        .pipe(gulp.dest("../../src/websrc/min/js/"))
        .pipe(gzip({ append: true }))
        .pipe(gulp.dest('../../data/js/'));
}

function imggz() {
    return gulp.src("../../src/websrc/img/*.*")
        .pipe(gzip({
            append: true
        }))
        .pipe(gulp.dest('../../data/img/'));
}

function htmlgz() {
    return gulp.src("../../src/websrc/html/*.html")
        .pipe(htmlmin({ collapseWhitespace: true, removeComments: true, removeRedundantAttributes: true }))
        .pipe(gulp.dest("../../src/websrc/min/html/"))
        .pipe(gzip({ append: true }))
        .pipe(gulp.dest('../../data/html/'));
}

function jsongz() {
    return gulp.src("../../src/websrc/json/*.json")
        .pipe(jsonminify())
        .pipe(gulp.dest("../../src/websrc/min/json/"))
        .pipe(gzip({ append: true }))
        .pipe(gulp.dest('../../data/json/'));
}

const styleTasks = gulp.series(stylesConcat);
const scriptTasks = gulp.series(scriptsgz);
const imgTasks = gulp.series(imggz);
const htmlTasks = gulp.series(htmlgz);
const jsonTasks = gulp.series(jsongz);

exports.xzg = gulp.parallel(styleTasks, scriptTasks, imgTasks, htmlTasks, jsonTasks);