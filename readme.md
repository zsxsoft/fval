Fval
=====================
[![Build Status](https://travis-ci.org/zsxsoft/fval.svg?branch=master)](https://travis-ci.org/zsxsoft/fval)
[![AppVeyor Build Status](https://ci.appveyor.com/api/projects/status/github/zsxsoft/fval)](https://ci.appveyor.com/project/zsxsoft/fval/branch/master)

Fval(say F-word to eval) extension used to disable unsafe functions/eval with E_FATAL.

Based on [taint](https://github.com/laruence/taint). See: http://www.laruence.com/2012/02/18/2560.html (Chinese)

## Requires
PHP 7+

## Features
1. Disable ``eval``
1. Disable unsafe functions: ``passthru`` ``exec`` ``system`` ``shell_exec`` ``proc_open`` ``popen``
1. Throw error when ``include`` or ``require`` a file generate by PHP.

## Build
### *nix
```bash
phpize
./configure
make
make install
```

### Windows

If you're using PHP 7.1 + Non Thread Safe + x64, run ``ci\appveyor.bat`` with Administrator, it will auto compile to ``php-sdk\phpdev\vc14\x64\php-7.1.8\x64\Release\php-fval.dll``. Otherwise, follow https://github.com/OSTC/php-sdk-binary-tools/ and configure with ``--enable-fval=shared``.

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
1. [ ] ``.phpt`` tests.
1. [ ] Dynamic calls detection.

## License

The PHP License