Fval
=====================

Fval(say F-word to eval) extension used to disable unsafe functions/eval with E_FATAL.

Based on [taint](https://github.com/laruence/taint). See: http://www.laruence.com/2012/02/18/2560.html (Chinese)

## Requires
PHP 7+

## Features
1. Disable ``eval``
1. Disable unsafe functions: ``passthru`` ``exec`` ``system`` ``shell_exec`` ``proc_open`` ``popen``
1. Throw error when ``include`` or ``require`` a file generate by PHP.

## Install

todo...

## Configure
```ini
fval.disable_eval = 1
fval.disable_functions = 1
fval.enable = 1
fval.error_level => 512
```

## Known issues
1. The file wrote by extensions, like ``imagejpeg`` in ``gd``, will not being recorded. That means the hacker can eval PHP code hide in somewhere like image EXIF.

## TODO
1. [ ] Use hook or ``ptrace`` to record all file writing.
1. [ ] TravisCI and AppVeyor configure.
1. [ ] ``.phpt`` tests.
1. [ ] Dynamic calls detection.

## License

The PHP License