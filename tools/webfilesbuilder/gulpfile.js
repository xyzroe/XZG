var gulp = require('gulp');
var fs = require('fs');
var concat = require('gulp-concat');
var gzip = require('gulp-gzip');
var flatmap = require('gulp-flatmap');
var path = require('path');
var htmlmin = require('gulp-htmlmin');
var uglify = require('gulp-uglify');
var pump = require('pump');
var purgecss = require('gulp-purgecss');
var cssnano = require('gulp-cssnano');
const jsonminify = require('gulp-jsonminify');
const rename = require('gulp-rename');

function stylesConcat() {
    return gulp.src(['../../src/websrc/css/custom.css', '../../src/websrc/css/bootstrap.min.css'])
        .pipe(concat({
            path: 'style.css',
            stat: {
                mode: 0666
            }
        }))
        //.pipe(purgecss({
        //    content: ['../../src/websrc/html/*.html', '../../src/websrc/js/*.js']
        //}))
        .pipe(cssnano({preset: 'advanced'}))
        .pipe(gulp.dest('../../src/websrc/min/css/'))
        .pipe(gzip({
            append: true
        }))
        .pipe(gulp.dest('../../src/websrc/gzipped/css/'));
}


function styles(cb) {
    var source = "../../src/websrc/gzipped/css/" + "style.css.gz";
    var destination = "../../src/webh/css/" + "style.css.gz.h";
    fs.mkdirSync(path.dirname(destination), { recursive: true });

    var wstream = fs.createWriteStream(destination);
    wstream.on('error', function (err) {
        console.log(err);
    });

    var data = fs.readFileSync(source);

    wstream.write('#define required_css_gz_len ' + data.length + '\n');
    wstream.write('const uint8_t required_css_gz[] PROGMEM = {')

    for (i = 0; i < data.length; i++) {
        if (i % 1000 == 0) wstream.write("\n");
        wstream.write('0x' + ('00' + data[i].toString(16)).slice(-2));
        if (i < data.length - 1) wstream.write(',');
    }
    wstream.write('\n};')
    wstream.end();
    cb();
}

function scriptsgz() {
    return gulp.src("../../src/websrc/js/*.js")
        .pipe(uglify())
        .pipe(gulp.dest("../../src/websrc/min/js/"))
        .pipe(gzip({ append: true }))
        .pipe(gulp.dest('../../src/websrc/gzipped/js/'));
}

function scripts() {
    return gulp.src("../../src/websrc/gzipped/js/*.*")
        .pipe(flatmap(function (stream, file) {
            var filename = path.basename(file.path);
            var directory = "../../src/webh/js/";
            fs.mkdirSync(directory, { recursive: true });
            var wstream = fs.createWriteStream(path.join(directory, filename + ".h"));
            wstream.on("error", function (err) {
                console.log(err);
            });
            var data = file.contents;
            wstream.write("#define " + filename.replace(/\.|-/g, "_") + "_len " + data.length + "\n");
            wstream.write("const uint8_t " + filename.replace(/\.|-/g, "_") + "[] PROGMEM = {")

            for (i = 0; i < data.length; i++) {
                if (i % 1000 == 0) wstream.write("\n");
                wstream.write('0x' + ('00' + data[i].toString(16)).slice(-2));
                if (i < data.length - 1) wstream.write(',');
            }
            wstream.write("\n};")
            wstream.end();
            return stream;
        }));
}

function fontgz() {
    return gulp.src("../../src/websrc/fonts/*.*")
        .pipe(gulp.dest("../../src/websrc/fonts/"))
        .pipe(gzip({
            append: true
        }))
        .pipe(gulp.dest('../../src/websrc/gzipped/fonts/'));
}

function fonts() {
    return gulp.src("../../src/websrc/gzipped/fonts/*.*")
        .pipe(flatmap(function (stream, file) {
            var filename = path.basename(file.path);
            var directory = "../../src/webh/fonts/";
            fs.mkdirSync(directory, { recursive: true });
            var wstream = fs.createWriteStream(path.join(directory, filename + ".h"));
            wstream.on("error", function (err) {
                console.log(err);
            });
            var data = file.contents;
            wstream.write("#define " + filename.replace(/\.|-/g, "_") + "_len " + data.length + "\n");
            wstream.write("const uint8_t " + filename.replace(/\.|-/g, "_") + "[] PROGMEM = {")

            for (i = 0; i < data.length; i++) {
                if (i % 1000 == 0) wstream.write("\n");
                wstream.write('0x' + ('00' + data[i].toString(16)).slice(-2));
                if (i < data.length - 1) wstream.write(',');
            }
            wstream.write("\n};")
            wstream.end();
            return stream;
        }));
}

function imggz() {
    return gulp.src("../../src/websrc/img/*.*")
        .pipe(gulp.dest("../../src/websrc/img/"))
        .pipe(gzip({
            append: true
        }))
        .pipe(gulp.dest('../../src/websrc/gzipped/img/'));
}

function imgs() {
    return gulp.src("../../src/websrc/gzipped/img/*.*")
        .pipe(flatmap(function (stream, file) {
            var filename = path.basename(file.path);
            var directory = "../../src/webh/img/";
            fs.mkdirSync(directory, { recursive: true });
            var wstream = fs.createWriteStream(path.join(directory, filename + ".h"));
            wstream.on("error", function (err) {
                console.log(err);
            });
            var data = file.contents;
            wstream.write("#define " + filename.replace(/\.|-/g, "_") + "_len " + data.length + "\n");
            wstream.write("const uint8_t " + filename.replace(/\.|-/g, "_") + "[] PROGMEM = {")

            for (i = 0; i < data.length; i++) {
                if (i % 1000 == 0) wstream.write("\n");
                wstream.write('0x' + ('00' + data[i].toString(16)).slice(-2));
                if (i < data.length - 1) wstream.write(',');
            }
            wstream.write("\n};")
            wstream.end();
            return stream;
        }));
}

function htmlgz() {
    return gulp.src("../../src/websrc/html/*.html")
        .pipe(htmlmin({ collapseWhitespace: true, removeComments: true, removeRedundantAttributes: true }))
        .pipe(gulp.dest("../../src/websrc/min/html/"))
        .pipe(gzip({ append: true }))
        .pipe(gulp.dest('../../src/websrc/gzipped/html/'));
}

function htmls() {
    return gulp.src("../../src/websrc/gzipped/html/*.*")
        .pipe(flatmap(function (stream, file) {
            var filename = path.basename(file.path);
            var directory = "../../src/webh/html/";
            fs.mkdirSync(directory, { recursive: true });
            var wstream = fs.createWriteStream(path.join(directory, filename + ".h"));
            wstream.on("error", function (err) {
                console.log(err);
            });
            var data = file.contents;
            wstream.write("#define " + filename.replace(/\.|-/g, "_") + "_len " + data.length + "\n");
            wstream.write("const uint8_t " + filename.replace(/\.|-/g, "_") + "[] PROGMEM = {")

            for (i = 0; i < data.length; i++) {
                if (i % 1000 == 0) wstream.write("\n");
                wstream.write('0x' + ('00' + data[i].toString(16)).slice(-2));
                if (i < data.length - 1) wstream.write(',');
            }
            wstream.write("\n};")
            wstream.end();
            return stream;
        }));
}

function jsongz() {
    return gulp.src("../../src/websrc/json/*.json")
        .pipe(jsonminify())
        .pipe(gulp.dest("../../src/websrc/min/json/"))
        .pipe(gzip({ append: true }))
        .pipe(gulp.dest('../../src/websrc/gzipped/json/'));
}


function jsons() {
    return gulp.src("../../src/websrc/gzipped/json/*.*")
        .pipe(flatmap(function (stream, file) {
            var filename = path.basename(file.path);
            var directory = "../../src/webh/json/";
            fs.mkdirSync(directory, { recursive: true });
            var wstream = fs.createWriteStream(path.join(directory, filename + ".h"));
            wstream.on("error", function (err) {
                console.log(err);
            });
            var data = file.contents;
            wstream.write("#define " + filename.replace(/\.|-/g, "_") + "_len " + data.length + "\n");
            wstream.write("const uint8_t " + filename.replace(/\.|-/g, "_") + "[] PROGMEM = {");

            for (var i = 0; i < data.length; i++) {
                if (i % 1000 == 0) wstream.write("\n");
                wstream.write('0x' + ('00' + data[i].toString(16)).slice(-2));
                if (i < data.length - 1) wstream.write(',');
            }
            wstream.write("\n};")
            wstream.end();
            return stream;
        }));
}


const styleTasks = gulp.series(stylesConcat, styles);
const scriptTasks = gulp.series(scriptsgz, scripts);
const fontTasks = gulp.series(fontgz, fonts);
const imgTasks = gulp.series(imggz, imgs);
const htmlTasks = gulp.series(htmlgz, htmls);
const jsonTasks = gulp.series(jsongz, jsons);

exports.default = gulp.parallel(styleTasks, scriptTasks, fontTasks, imgTasks, htmlTasks, jsonTasks);
