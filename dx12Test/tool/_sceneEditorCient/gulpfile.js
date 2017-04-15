var gulp = require('gulp');
var browserify = require('browserify');
var source = require('vinyl-source-stream');
var riotify = require('riotify');

gulp.task('browserify', function(){
  var b = browserify({
    entries: ['./src/app.js'],
    transform: [riotify]
  });
  return b.bundle()
    .pipe(source('app.js'))
    .pipe(gulp.dest('./dest'));
});
