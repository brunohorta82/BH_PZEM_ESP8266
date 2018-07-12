/**
 * @file gulpfile.js
 * @description Gulp task automator javascript file.
 * @author Bruno Horta
 * @email brunohorta82@gmail.com
 */
const gulp = require('gulp'),
    fs = require('fs'),
    exec = require('child_process').exec,

    // Browser Sync
    browserSync = require('browser-sync').create();

const SRC_FOLDER = './';
const SRC_ASSETS_FOLDER = SRC_FOLDER + '/assets';

/**
 * Serve task.
 * This task is responsible for launching Browser Sync and setting up watchers over the file types involved in the
 * development process. If any changes are detected in one of those files, the build process is triggered and subsequently
 * Browser Sync reloads the application in all connected browsers.
 */
gulp.task('serve', function () {
    // make sure the application is built before launching
    fs.stat(SRC_FOLDER + '/index.html', function (err) {
        if (!err) {
            browserSync.init({
                server: {
                    baseDir: SRC_FOLDER,
                    index: 'index.html'
                }
            });
            // listen for changes in the following file types
            gulp.watch([SRC_FOLDER + '/**/*.js', SRC_FOLDER + '/**/*.html', SRC_FOLDER + '/**/*.css']).on('change', browserSync.reload);
        } else {
            // detect specific errors
            switch (err.code) {
                case 'ENOENT':
                    console.log('\x1b[31mGulp "serve" task error\x1b[0m: There is no build available. ' +
                        'Please, run the command \x1b[32mgulp build\x1b[0m before starting the server ' +
                        'or simply \x1b[32mgulp\x1b[0m.\n');
                    break;
                default:
                    console.log('\x1b[31mGulp "serve" task error\x1b[0m: Unknown error. Details: ', err);
                    break;
            }
        }

    });
});

/**
 * Default task.
 * This task is responsible for bundling and running all tasks associated with the production of the application
 * in a distributable format. This task also starts the application server in development mode.
 */
gulp.task('default', function () {
    gulp.start('serve');
});